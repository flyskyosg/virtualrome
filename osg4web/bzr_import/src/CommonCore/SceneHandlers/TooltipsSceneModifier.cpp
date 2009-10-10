

#include <CommonCore/Scenehandlers/TooltipsSceneModifier.h>

#include <vector>

#include <osg/Camera>
#include <osg/Geometry>


using namespace SceneHandlers;

/********************************************************************
 *
 *  Metodi Classe TooltipsSceneModifier
 *
 ********************************************************************/

/** Costruttore del TooltipsSceneModifier */

TooltipsSceneModifier::TooltipsSceneModifier(float initx, float inity) : CommandStubb("show_tooltip"), //TODO: spostare in defines
	_xInitialResolution(initx),
	_yInitialResolution(inity),
	_xCurrentResolution(_xInitialResolution),
	_yCurrentResolution(_yInitialResolution),
	_hudgrp(new osg::Group),
	_matrtrans(new osg::MatrixTransform),
	_switchnd(new osg::Switch),
	_textnd(new osgText::Text),
	_geombox(new osg::Geometry),
	_geodend(new osg::Geode),
	_currentactivetooltip(NULL),
	_negatemask(0xfffffffe),
	_allowmask(0x0000001),
	_startTime(0),
	_maxTime(500.0) //TODO:
{
	this->createTooltipSceneGraph();
}

TooltipsSceneModifier::~TooltipsSceneModifier()
{

}

osg::Node* TooltipsSceneModifier::createTooltipHUD()
{
	_hudgrp->setName("TooltipsSceneModifier_HUD_Group");
	_hudgrp->setNodeMask(_negatemask);

	this->createTooltipCamera2D();

	return (osg::Node*) _hudgrp.get();
}

void TooltipsSceneModifier::createTooltipSceneGraph()
{
	//Inizializzo la Matrice di Posizionamento del Tooltip nella screen 2D
	_matrtrans->setMatrix(osg::Matrix::identity());
	_matrtrans->setName("Tooltips_Matrice_di_Traslazione");

	//Inizializzo il testo
	_textnd->setName("Tooltip_Testo");
	_textnd->setFont("fonts/arial.ttf"); //FIXME
    _textnd->setCharacterSize(15.0);
	_textnd->setPosition(osg::Vec3(0.0,0.0,0.0));
	_textnd->setColor(osg::Vec4(1.0f, 153.0f / 255.0f, 0.0f, 1.0f)); //FIXME: ripristinare

	_textnd->setBackdropType(osgText::Text::OUTLINE);
    _textnd->setBackdropImplementation(osgText::Text::POLYGON_OFFSET);
    _textnd->setBackdropOffset(0.025f);
    _textnd->setBackdropColor(osg::Vec4(176.0f / 255.0f, 106.0f / 255.0f, 0.0f, 1.0f));

	//Disabilito il modello di illuminazione per i tooltip
	osg::ref_ptr<osg::StateSet> globstateset = _geodend->getOrCreateStateSet();
	globstateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	//Teso e Geometria dello sfondo sotto Geode
	_geodend->setName("Tooltip_Backgroud");
	_geodend->addDrawable(_textnd.get());
	_geodend->addDrawable(_geombox.get());

	//Geode su MatrixTransform
	_matrtrans->addChild(_geodend.get());

	//MT su NodoSwitch di accensione/spegnimento del tooltip
	_switchnd->setName("Tooltip_Switch");
	_switchnd->addChild(_matrtrans.get(), false);
}

void TooltipsSceneModifier::createTooltipCamera2D()
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	
	//Inizializzo la camera di proiezione 2D
	camera->setName("Tooltip_HUD_Camera_2D");
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, 1280, 0, 1024)); 
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setAllowEventFocus(false);

	//Switch su Camera 2D
	camera->addChild(_switchnd.get());

	//Camera 2D attaccata alla scena
	_hudgrp->addChild(camera.get());
}

/** Spenge la visualizzazione del Tooltip */
void TooltipsSceneModifier::switchOffTooltip()
{
	//Resetto la copia di controllo tooltip
	_currentactivetooltip = NULL;
	//Spengo la visualizzazione del tooltip
	_switchnd->setAllChildrenOff();
	//Resetto il counter
	_startTime = 0;
}

/** Inizializza e Attiva il Tooltip */
void TooltipsSceneModifier::switchOnTooltip(std::string argument)
{
	//Rimuovo le geometry di sfondo per calcolare la bbox del testo
	_geodend->removeDrawable(_geombox.get());

	//Setto il nuovo testo della tooltip
	_textnd->setText(argument);
	
	//Ricalcolo la BBox del testo
	osg::BoundingBox bb;
	for(unsigned int i = 0; i < _geodend->getNumDrawables(); ++i)
		bb.expandBy(_geodend->getDrawable(i)->getBound());
	
	//Creo lo sfondo in base alla BBox del testo
	createTooltipBackgroud(bb);

	//Attacco lo sfondo al geode
	_geodend->addDrawable(_geombox.get());

	//Visualizzo il tooltip
	_switchnd->setAllChildrenOn();
}

/** Creo la geometria di sfondo al testo del Tooltip in base alla BBox */

void TooltipsSceneModifier::createTooltipBackgroud(osg::BoundingBox bb)
{
	float depth = bb.zMin() - 0.1; //Profondità
	float scar = 0.1; //Adder sulla dimensione della BBox

	//Calcolo la BBox con aggiunta dell'Adder
	float deltax = abs( abs( bb.xMin()) - abs(bb.xMax() ) ) * ( scar / 10.0f); //FIXME: / 10.f prova
	float deltay = abs( abs( bb.yMin()) - abs(bb.yMax() ) ) * scar;
	bb.set(bb.xMin() - deltax, bb.yMin() - deltay, bb.zMin(), bb.xMax() + deltax, bb.yMax() + deltay, bb.zMax());

	//Creo il rettangolo 
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	vertices->push_back(osg::Vec3(bb.xMin(),bb.yMax(),depth));
	vertices->push_back(osg::Vec3(bb.xMin(),bb.yMin(),depth));
	vertices->push_back(osg::Vec3(bb.xMax(),bb.yMin(),depth));
	vertices->push_back(osg::Vec3(bb.xMax(),bb.yMax(),depth));
	_geombox->setVertexArray(vertices.get()); 

	//Associo Normali
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
	_geombox->setNormalArray(normals.get());
	_geombox->setNormalBinding(osg::Geometry::BIND_OVERALL);

	//Associo Colore
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f, 0.35f)); // FIXME: ripristinare osg::Vec4(1.0f,1.0,0.8f,0.2f)); 
	_geombox->setColorArray(colors.get());
	_geombox->setColorBinding(osg::Geometry::BIND_OVERALL);

	//Definisco la primitiva per la geometria
	_geombox->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

	//Nuovo Stateset di attivazione BLEND
	osg::ref_ptr<osg::StateSet> stateset = _geombox->getOrCreateStateSet();
	stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
	//stateset->setAttribute(new osg::PolygonOffset(1.0f,1.0f),osg::StateAttribute::ON);

	//Passo al RenderBin di trasparenza
	stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

/** Setta il nodo per ricevere l'evento tooltip, ritorna nodo da attaccare alla scena */
osg::Node* TooltipsSceneModifier::setTooltipsProperties(osg::Node* node, std::string descstr)
{
	/************************************************************************
	 *
	 * Per far funzionare il Tooltip Handler si deve aggiungere nei nodi:
	 *
	 * Campo DESCRIPTIONLIST(n): "show_tooltip 'testo da visualizzare nel tooltip'"
	 */

	osg::ref_ptr<osg::Group> grp = new osg::Group;

	std::string totstr = this->getCommand() + " " + descstr;

	osg::Node::DescriptionList desclist = grp->getDescriptions();
	desclist.push_back(totstr);

	grp->setDescriptions(desclist);
	grp->setName("Nodo_Parsing_Tooltip");
	grp->addChild(node);
	grp->setNodeMask( _allowmask );

	return grp.release();
}

#include <iostream>
/** Handling evento show_tooltip */

bool TooltipsSceneModifier::commandBridge(std::string argument, osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	switch(ea.getEventType())
	{
	case osgGA::GUIEventAdapter::FRAME:
		{
			if(node != _currentactivetooltip) //c'è intersezione con nodo diverso. Setto Tooltip con nuovo testo
			{
				_currentactivetooltip = node;
				switchOnTooltip(argument);
			}

			// Increase resolution
			osgViewer::Viewer::Windows windows;

			viewer->getWindows(windows);
			for(osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
			{
				int x, y, width, height;
				(*itr)->getWindowRectangle(x, y, width, height);

				_xCurrentResolution = width;
				_yCurrentResolution = height;

				osg::ref_ptr<osg::Camera> cam = dynamic_cast<osg::Camera* >( _hudgrp->getChild(0) );

				if(cam.valid())
					cam->setProjectionMatrixAsOrtho2D(0, _xCurrentResolution, 0, _yCurrentResolution);
			}
		
			moveTooltip(ea); //Aggiorno la posizione della MT

			//Setto il timer di start
			_startTime = osg::Timer::instance()->tick();
		}
	case osgGA::GUIEventAdapter::RESIZE:
		{
			
		}
		break;
	default:
		break;
	}

	return false; //Consento agli altri parser di continuare
}


/** Aggiorna la MT del tooltip in base alle coordinate del mouse */

void TooltipsSceneModifier::moveTooltip(const osgGA::GUIEventAdapter& ea)
{
	float wratio = 1.0f;
	float hratio = 1.0f;

	if(_xInitialResolution != 0 && _xCurrentResolution != 0)
		wratio = _xInitialResolution / _xCurrentResolution;

	if(_yInitialResolution != 0 && _yCurrentResolution != 0)
		hratio = _yInitialResolution / _yCurrentResolution;

	//Calcolo la posizione effettiva del tooltip (in coordinate Camera 2D) in base alla posizione del mouse sulla finestra
	float x = (ea.getXnormalized() * wratio + 1.0f) * 0.5f * _xCurrentResolution;
	float y = (ea.getYnormalized() * hratio + 1.0f) * 0.5f * _yCurrentResolution;
	
	//Setto nuova matrice
	_matrtrans->setMatrix(osg::Matrix::identity() * osg::Matrix::translate(osg::Vec3(x, y, 0.0)) );
}


/** Timer di mantenimento delle labels */

void TooltipsSceneModifier::checkTooltipsTiming()
{
	if(_startTime != 0)
		if(osg::Timer::instance()->tick() > _startTime + _maxTime)
			switchOffTooltip();
}