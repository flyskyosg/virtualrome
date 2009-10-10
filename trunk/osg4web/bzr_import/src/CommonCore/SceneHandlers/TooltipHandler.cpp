

#include <CommonCore/Scenehandlers/TooltipHandler.h>

#include <vector>

#include <osg/Camera>
#include <osg/Geometry>


using namespace SceneHandlers;

/********************************************************************
 *
 *  Metodi Classe Tooltip Handler
 *
 ********************************************************************/

/** Costruttore del ToolTips Handler */


TooltipHandler::TooltipHandler(osg::Node::NodeMask nm) : NodeParserHandler(nm),
	_tooltipcommandname("tooltip"), //TODO: spostare in defines
	_matrtrans(new osg::MatrixTransform),
	_switchnd(new osg::Switch),
	_textnd(new osgText::Text),
	_geombox(new osg::Geometry),
	_geodend(new osg::Geode),
	_currentactivetooltip(NULL),
	_xMaxResolution(500), //TODO:
	_yMaxResolution(500), //TODO:
	_hudgrp(NULL)
{
	this->createTooltipSceneGraph();
	
	this->registerCommand(_tooltipcommandname);
}


TooltipHandler::TooltipHandler(osg::Group* hudgrp, osg::Node::NodeMask nm) : NodeParserHandler(nm),
	_tooltipcommandname("tooltip"), //TODO: spostare in defines
	_matrtrans(new osg::MatrixTransform),
	_switchnd(new osg::Switch),
	_textnd(new osgText::Text),
	_geombox(new osg::Geometry),
	_geodend(new osg::Geode),
	_currentactivetooltip(NULL),
	_xMaxResolution(500), //TODO:
	_yMaxResolution(500), //TODO:
	_hudgrp(hudgrp)
{
	this->createTooltipSceneGraph();
	this->createTooltipCamera2D();

	this->registerCommand(_tooltipcommandname);
}

/** Setta il nodo di scena globale nel Tooltip */
void TooltipHandler::setMainSceneNode(osg::Node* node)
{
	_hudgrp = (osg::Group*) node;

	this->createTooltipCamera2D();
}

void TooltipHandler::createTooltipSceneGraph()
{
	//Inizializzo la Matrice di Posizionamento del Tooltip nella screen 2D
	_matrtrans->setMatrix(osg::Matrix::identity());
	_matrtrans->setName("Tooltip: Matrice di Traslazione");

	//Inizializzo il testo
	_textnd->setName("Tooltip: Testo");
	_textnd->setFont("fonts/arial.ttf"); //FIXME
    _textnd->setCharacterSize(15.0);
	_textnd->setPosition(osg::Vec3(0.0,0.0,0.0));

	//Disabilito il modello di illuminazione per i tooltip
	osg::ref_ptr<osg::StateSet> globstateset = _geodend->getOrCreateStateSet();
	globstateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	//Teso e Geometria dello sfondo sotto Geode
	_geodend->setName("Tooltip: Backgroud");
	_geodend->addDrawable(_textnd.get());
	_geodend->addDrawable(_geombox.get());

	//Geode su MatrixTransform
	_matrtrans->addChild(_geodend.get());

	//MT su NodoSwitch di accensione/spegnimento del tooltip
	_switchnd->setName("Tooltip: Switch");
	_switchnd->addChild(_matrtrans.get(), false);
}

void TooltipHandler::createTooltipCamera2D()
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	
	//Inizializzo la camera di proiezione 2D
	camera->setName("Tooltip: HUD Camera 2D");
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, _xMaxResolution, 0, _yMaxResolution)); 
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

/** Riscrittura della gestione dell'evento FRAME */

void TooltipHandler::callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
{
	//Richiamo la sotto funzione per la gestione del Parsing del NodePath intersecato
	NodeParserHandler::callmethodqueue(viewer, ea);

	//Controllo se esistono comandi di tooltip in coda
	if(this->getNodeCommandListSize() != 0)
	{
		//Prendo il primo comando in coda 
		NodeCommandList::NodeCommand ndc;

		if(this->findNodeCommandByName("tooltip", ndc))
		{
			if(ndc.getNode() != _currentactivetooltip) //c'è intersezione con nodo diverso. Setto Tooltip con nuovo testo
			{
				_currentactivetooltip = ndc.getNode();
				switchOnTooltip(ndc, ea);
			}

			moveTooltip(ea); //Aggiorno la posizione della MT
		}
		else
			switchOffTooltip();
	}
	else
		switchOffTooltip(); //nessun comando per il nodepath di intersezione



	NodeCommandList::NodeCommand ndc;

	/*
	if(this->findNodeCommandByRegister(ndc))
	{
		if(ndc.getNode() != _currentactivetooltip) //c'è intersezione con nodo diverso. Setto Tooltip con nuovo testo
		{
			_currentactivetooltip = ndc.getNode();
			switchOnTooltip(ndc, ea);
		}
		moveTooltip(ea); //Aggiorno la posizione della MT
	}
	else
		switchOffTooltip();
	*/
}

/** Spenge la visualizzazione del Tooltip */

void TooltipHandler::switchOffTooltip()
{
	//Resetto la copia di controllo tooltip
	_currentactivetooltip = NULL;
	//Spengo la visualizzazione del tooltip
	_switchnd->setAllChildrenOff();
}

/** Inizializza e Attiva il Tooltip */

void TooltipHandler::switchOnTooltip(NodeCommandList::NodeCommand ndc, const osgGA::GUIEventAdapter& ea)
{
	//Rimuovo le geometry di sfondo per calcolare la bbox del testo
	_geodend->removeDrawable(_geombox.get());

	//Richiamo il testo per il tooltip dal NodeCommand
	std::string text;
	std::vector<std::string> desclist;

	if(ndc.getDescListWithoutCommand(desclist))
	{
		for(unsigned int i = 0; i < desclist.size(); i++)
		{
			text += desclist.at(i);
			text += "\n";
		}
	}
	
	//Setto il nuovo testo della tooltip
	_textnd->setText(text);
	
	//Ricalcolo la BBox del testo
	osg::BoundingBox bb;
	for(unsigned int i = 0; i < _geodend->getNumDrawables(); ++i)
	{
		bb.expandBy(_geodend->getDrawable(i)->getBound());
	}

	//Creo lo sfondo in base alla BBox del testo
	createTooltipBackgroud(bb);

	//Attacco lo sfondo al geode
	_geodend->addDrawable(_geombox.get());

	//Visualizzo il tooltip
	_switchnd->setAllChildrenOn();
}

/** Aggiorna la MT del tooltip in base alle coordinate del mouse */

void TooltipHandler::moveTooltip(const osgGA::GUIEventAdapter& ea)
{
	//Calcolo la posizione effettiva del tooltip (in coordinate Camera 2D) in base alla posizione del mouse sulla finestra
	float x = (ea.getXnormalized()+1.0f) * 0.5f * _xMaxResolution;
	float y = (ea.getYnormalized()+1.0f) * 0.5f * _yMaxResolution;
	
	//Setto nuova matrice
	_matrtrans->setMatrix(osg::Matrix::identity() * osg::Matrix::translate(osg::Vec3(x, y, 0.0)) );
}

/** Creo la geometria di sfondo al testo del Tooltip in base alla BBox */

void TooltipHandler::createTooltipBackgroud(osg::BoundingBox bb)
{
	float depth = bb.zMin() - 0.1; //Profondità
	float scar = 0.1; //Adder sulla dimensione della BBox

	//Calcolo la BBox con aggiunta dell'Adder
	float deltax = abs( abs( bb.xMin()) - abs(bb.xMax() ) ) * scar;
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
	colors->push_back(osg::Vec4(1.0f,1.0,0.8f,0.2f)); 
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
osg::Node* TooltipHandler::setTooltipsProperties(osg::Node* node, osg::Node::DescriptionList desclist)
{
	/************************************************************************
	 *
	 * Per far funzionare il Tooltip Handler si deve aggiungere nei nodi:
	 *
	 * Campo DESCRIPTIONLIST(0): "tooltip"
	 * Campo DESCRIPTIONLIST(n): testo da visualizzare nel tooltip
	 */

	osg::ref_ptr<osg::Group> grp = new osg::Group;

	osg::Node::DescriptionList ndesclist;
	ndesclist.push_back(_tooltipcommandname);

	osg::Node::DescriptionList::iterator itr = desclist.begin();
	for( ; itr != desclist.end(); itr++ )
		ndesclist.push_back(*itr);

	grp->setDescriptions(ndesclist);
	grp->setName("Nodo Parsing Tooltip");
	grp->addChild(node);
	grp->setNodeMask( this->getTraversalNodeMask() );
	
	return grp.release();
}