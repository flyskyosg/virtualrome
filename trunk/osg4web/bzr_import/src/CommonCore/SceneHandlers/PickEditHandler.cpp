#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/CoordinateSystemNode>
#include <osgText/Text>

#include <osgGA/TrackballManipulator>

#include <osgManipulator/CommandManager>
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabPlaneDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/Translate1DDragger>
#include <osgManipulator/Translate2DDragger>
#include <osgManipulator/TranslateAxisDragger>

#include <iostream>
#include <sstream>


#include <osg/ShapeDrawable>

#include <CommonCore/SceneHandlers/PickEditHandler.h>


using namespace SceneHandlers;

bool PickEditHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa,
                    osg::Object*, osg::NodeVisitor*)
		{
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (!view) return false;
			
						
			//TODO: Finire Billboard

			//premendo SHIFT-DX decido se attaccare un modello o una billboard
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Shift_R &&
				ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN &&
                _activeDragger == 0 && _waitingCommit == false)
			{
				_model_billboard = !_model_billboard;
				if(_model_billboard)
					std::cout << "Attacco modello" << std::endl;
				else
					std::cout << "Attacco billboard" << std::endl;

			}

			///////////////////////////////
			if(_mode == VIEW) return false;
			///////////////////////////////
			

			////////////////
			if(_mode == ADD)
			////////////////
			{
				switch (ea.getEventType())
				{
				case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
					{
	
						osgUtil::LineSegmentIntersector::Intersections intersections;

						if(view->computeIntersections(ea.getX(), ea.getY(), intersections))
						{
							for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
								hitr != intersections.end();
								++hitr)
							{
								//std::cout << "Sono nel ciclo for..." << std::endl;	
							
								if (!hitr->nodePath.empty()  /*&& !(hitr->nodePath.back()->getName().empty())*/ )
								{
									int i;
									int j = hitr->nodePath.size()-1;
									//std::cout << "j = " << j << std::endl;
									for(i=0;i<=j;i++)
									{
										osg::ref_ptr<osg::Node> node = hitr->nodePath.at(i);
				
										if(node.valid() && node->getName() == "terreni" )
										{
												position = hitr->getWorldIntersectPoint();
												//position.set( position.x(), position.y(), position.z() + 100 );

												if(_model_billboard)
												{
													_add_node = true;
													//std::cout << "Intersezione...OK" << std::endl;
												}

												else
												{
													_add_billboard = true;
													//std::cout << "Intersezione...OK" << std::endl;
												}
												
												return true;
										}

									}
								}
							}
						}
						return true;
					}
				}
			}
			
			////////////////////
			if (_mode == SELECT)
			////////////////////
			{
				switch (ea.getEventType())
				{
				case osgGA::GUIEventAdapter::PUSH:
					{
	
						osgUtil::LineSegmentIntersector::Intersections intersections;

						if(view->computeIntersections(ea.getX(), ea.getY(), intersections))
						{
							for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
								hitr != intersections.end();
								++hitr)
							{
													
								if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
								{
									int i;
									int j = hitr->nodePath.size()-1;
									for(i=0;i<=j;i++)
									{
										osg::ref_ptr<osg::Node> node = hitr->nodePath.at(i);
										
										if(node.valid() && node->getName() == "pickable")
										{
											
											osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(node.get());
											if(mt.valid())
											{
												parentNode = mt.get();
												childNode = mt->getChild(0);

												_ready_to_remove = true;
												_waitingCommit = true;
												// _mode = PICK;
												this->setMode(3); //PICK
												break;
													
											}
											
										}

									}
								}
							}
						}
					}
				}
			  
			}
						
			//////////////////
			if (_mode == PICK)
			//////////////////
			{
			
				switch (ea.getEventType())
				{
					case osgGA::GUIEventAdapter::PUSH:
					{
						osgUtil::LineSegmentIntersector::Intersections intersections;

						_pointer.reset();

						if (view->computeIntersections(ea.getX(),ea.getY(),intersections))
						{
							_pointer.setCamera(view->getCamera());
							_pointer.setMousePosition(ea.getX(), ea.getY());

							for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
								hitr != intersections.end();
								++hitr)
							{
								_pointer.addIntersection(hitr->nodePath, hitr->getLocalIntersectPoint());
							}
							for (osg::NodePath::iterator itr = _pointer._hitList.front().first.begin();
								itr != _pointer._hitList.front().first.end();
								++itr)
							{
								osgManipulator::Dragger* dragger = dynamic_cast<osgManipulator::Dragger*>(*itr);
								if (dragger)
								{

									dragger->handle(_pointer, ea, aa);
									_activeDragger = dragger;
									break;
								}                   
							}
						}
					}
				
			
					case osgGA::GUIEventAdapter::DRAG:
					case osgGA::GUIEventAdapter::RELEASE:
					{
						if (_activeDragger)
						{
							_pointer._hitIter = _pointer._hitList.begin();
							_pointer.setCamera(view->getCamera());
							_pointer.setMousePosition(ea.getX(), ea.getY());

							_activeDragger->handle(_pointer, ea, aa);
						}
						break;
					}
				
					default:
						break;
				}
        
			}

            if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
            {
                _activeDragger = 0;
                _pointer.reset();
            }

            return true;
		}

bool PickEditHandler::removeNode(osg::Node* child, osg::Node* parent)
{
	osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(parent);

	if(mt.valid())
	{
		mt->removeChild( child );
		return true;
	}
	else
		return false;
}



osg::Node* PickEditHandler::getChildNode()
{
	return childNode.get();
}



osg::Node* PickEditHandler::getParentNode()
{
	return parentNode.get();
}

		

void PickEditHandler::resetCommitFlag()
{
	_commit = false;
}

		

void PickEditHandler::resetRemoveFlag()
{
	_ready_to_remove = false;
}

		
bool PickEditHandler::commitStatus()
{
	return _commit;
}
		
		
bool PickEditHandler::removeStatus()
{
	return _ready_to_remove;
}

		
void PickEditHandler::resetWaitingCommit()
{
	_waitingCommit = false;
}

		
osgManipulator::CommandManager* PickEditHandler::getCmdMgr()
{
	return _cmdMgr.get();
}

		
std::string PickEditHandler::getDraggerName()
{
	return _dragger_name;
}

		
osg::Node* PickEditHandler::getSelectionNode()
{
	return selection;
}

		
void PickEditHandler::setActionView()
{
	this->setMode(0); //VIEW
}

		

void PickEditHandler::resetPointers()
{
	parentNode = 0;
	childNode = 0;
	selection = 0;
}

		

osgManipulator::Dragger* PickEditHandler::createDragger(const std::string& name)
{
	osgManipulator::Dragger* dragger = 0;
	if ("TabPlaneDragger" == name)
	{
		osgManipulator::TabPlaneDragger* d = new osgManipulator::TabPlaneDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}
	else if ("TabPlaneTrackballDragger" == name)
	{
		osgManipulator::TabPlaneTrackballDragger* d = new osgManipulator::TabPlaneTrackballDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}
	else if ("TrackballDragger" == name)
	{
		osgManipulator::TrackballDragger* d = new osgManipulator::TrackballDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}
	else if ("Translate1DDragger" == name)
	{
		osgManipulator::Translate1DDragger* d = new osgManipulator::Translate1DDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}
	else if ("Translate2DDragger" == name)
	{
		osgManipulator::Translate2DDragger* d = new osgManipulator::Translate2DDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}
	else if ("TranslateAxisDragger" == name)
	{
		osgManipulator::TranslateAxisDragger* d = new osgManipulator::TranslateAxisDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}
	else //default o stringa non riconosciuta
	{
		osgManipulator::TabBoxDragger* d = new osgManipulator::TabBoxDragger();
		d->setupDefaultGeometry();
		dragger = d;
	}

	return dragger;
}



osg::Node* PickEditHandler::addDraggerToScene(osg::Node* nodo, osgManipulator::CommandManager* cmdMgr, const std::string& name)
{
	nodo->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

	selection = new osgManipulator::Selection;

	selection->addChild(nodo);

	osgManipulator::Dragger* dragger = createDragger(name);

	osg::Group* root = new osg::Group;
	root->addChild(dragger);
	root->addChild(selection);

	float scale = nodo->getBound().radius() * 1.6;
	dragger->setMatrix(osg::Matrix::scale(scale, scale, scale) *
               osg::Matrix::translate(nodo->getBound().center()));
	cmdMgr->connect(*dragger, *selection);

	return root;
}

bool PickEditHandler::removeGizmo( osg::Node* model, osg::Node* parent, osg::Node* selection )
{
	osg::ref_ptr<osg::MatrixTransform> mt_parent = dynamic_cast<osg::MatrixTransform*>(parent);

	osg::ref_ptr<osg::MatrixTransform> sel = dynamic_cast<osgManipulator::Selection*>(selection);


	if( mt_parent.valid() && sel.valid() )
	{
		//attacco al parent il nodo
		mt_parent->addChild( model );

		//faccio il commit delle modifiche al modello
		osg::Matrix m;

		m = sel->getMatrix();

		//con preMult non funziona perchè è sbagliato il calcolo percorrendo l'albero da sopra a sotto
		m.postMult( mt_parent->getMatrix() );

		mt_parent->setMatrix( m );
		
		if(mt_parent->getNumChildren() > 1)
		{
			mt_parent->removeChildren(0,1);	//rimuove il primo figlio (root del gizmo)	
			sel->removeChild( model );
		}
		
		else
			return false;

	}

	return true;
}


osg::Vec3 PickEditHandler::getPosition()
{
	return position;
}


bool PickEditHandler::readyToAdd()
{
	return _add_node;
}


bool PickEditHandler::readytoAddBillboard()
{
	return _add_billboard;
}


void PickEditHandler::resetFlag()
{
	_add_node = false;
	_add_billboard = false;
}


std::string PickEditHandler::getModel()
{
	return _model;
}


std::string PickEditHandler::getBillBoardImageName()
{
	return _bbimagename;
}


void PickEditHandler::storeHUDPointer(osg::Group* pHud)
{
	osg::ref_ptr<osg::Geode> hud_ptr = dynamic_cast<osg::Geode*>(pHud->getChild(0));

	textMode    = dynamic_cast<osgText::Text*>(hud_ptr->getDrawable(0));
	textDragger = dynamic_cast<osgText::Text*>(hud_ptr->getDrawable(1));
	textModel   = dynamic_cast<osgText::Text*>(hud_ptr->getDrawable(2));
}


void PickEditHandler::defaultText()
{
	
	textMode->setText("Modalita' corrente >> VIEW");		
	textDragger->setText("Dragger corrente >> TabBoxDragger");
	textModel->setText("Modello corrente >> cow.osg");		
}


//mappatura comandi per JS
bool PickEditHandler::setMode(unsigned int mode)
{
	_mode = mode;

	return true;
}

bool PickEditHandler::setModel(std::string model)
{
	_model = model;

	return true;
}

bool PickEditHandler::setBillBoardImageName(std::string bimage)
{
	_bbimagename = bimage;

	//TODO: BillBoard come Model

	return true;
}

bool PickEditHandler::setDragger(std::string dragger)
{
	_dragger_name = dragger;

	return true;
}



// Ridefinisco la funzione che gestisce i comandi
std::string PickEditHandler::handleAction(std::string argument)
{
	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);

	switch(this->getCommandActionIndex(lcommand))
	{
	case 0: //VIEW
		if( !this->setMode(0) )
			retstr = "CORE_FAILED";
		break;
	case 1: //ADD
		if( !this->setMode(1) )
			retstr = "CORE_FAILED";
		break;
	case 2: //SELECT
		if( !this->setMode(2) )
			retstr = "CORE_FAILED";
		break;
	case 3:	//CHANGE_MODEL
		if( !this->setModel(rcommand) )
			retstr = "CORE_FAILED";
		break;
	case 4:	//CHANGE_DRAGGER
		if( !this->setDragger(rcommand) )
			retstr = "CORE_FAILED";
		break;
	case 5:	//CHANGE_BILLBOARD
		if( !this->setBillBoardImageName(rcommand) )
			retstr = "CORE_FAILED";
		break;
	case 6: //COMMIT
		if(_waitingCommit == true)
			_commit = true;
		break;
	default:
		break;

	}
	
	return retstr;
}


void PickEditHandler::handleSceneGraph()
{
	if( this->removeStatus() )
	{
		this->removeNode(this->getChildNode(), this->getParentNode());
			
		osg::ref_ptr<osg::MatrixTransform> mt = dynamic_cast<osg::MatrixTransform*>(this->getParentNode());

		if(mt.valid())
		{
			mt->addChild( this->addDraggerToScene( this->getChildNode(), 
				this->getCmdMgr(), 
				this->getDraggerName() ) ); 
		}

		this->resetRemoveFlag();
	}

	if( this->commitStatus() )
	{
		//operazioni di staccamento del gizmo
		this->removeGizmo( this->getChildNode(), this->getParentNode(), this->getSelectionNode());
		
		//passo alla modalità VIEW
		this->setActionView();
		this->resetWaitingCommit();		//resetto il flag per nessun gizmo presente che aspetta la commit 
		this->resetCommitFlag();		//commit eseguita, resetto
		this->resetPointers();
	}

	if( this->readyToAdd() )
	{	
		//aggiunta nodo
		//std::cout << "Entrato in routine di aggiunta nodo" << std::endl;
	
		osg::ref_ptr<osg::MatrixTransform> model_mt = new osg::MatrixTransform;
		model_mt->setName("pickable");	//per fare in modo che dopo sia modificabile
		osg::ref_ptr<osg::Node> model = osgDB::readNodeFile( this->getModel() );

		if(!model.valid())
		{
			osg::TessellationHints* hints = new osg::TessellationHints;
			hints->setDetailRatio(0.5f);

			osg::ref_ptr<osg::Geode> ggeo = new osg::Geode;
			ggeo->addDrawable(new osg::ShapeDrawable(new osg::Box( osg::Vec3(0.0f,0.0f,0.0f), 10.0f), hints));
			model = (osg::Node*) ggeo.get(); 
		}

		osg::Matrix model_matrix;
		//osg::Matrix model_scale;
		osg::Matrix model_translate;
		//scala per terreno campania, non serve altrimenti
		//model_scale.makeScale(100, 100, 100);
		//[da mettere a posto] modifico la coordinata z in base al bounding box del modello
		osg::Vec3 pos_finale;
		osg::BoundingSphere bs = model->getBound();
		pos_finale.set( this->getPosition() );
		
		pos_finale.set( pos_finale.x(), pos_finale.y(), pos_finale.z() + ( bs.radius()*0.4 ) );
		model_translate.makeTranslate( pos_finale );
		
		//PRIMA trasla, POI scala (studiare la geometria serviva...). Scala se serve...
		model_matrix.preMult( model_translate );
		//model_matrix.preMult( model_scale );
				
		model_mt->setMatrix( model_matrix );		
		model_mt->addChild( model.get() );
		_added_models->addChild( model_mt.get() );
		this->resetFlag();
	}
	if( this->readytoAddBillboard() )
	{
		//attacca billboard
		//std::cout << "Entrato in routine di aggiunta billboard" << std::endl;
	
		osg::ref_ptr<osg::MatrixTransform> bb_mt = new osg::MatrixTransform;
		bb_mt->setName( "pickable" );	//per fare in modo che dopo sia modificabile
		osg::ref_ptr<osg::Image> image = osgDB::readImageFile( _bbimagename );

		if(!image.valid())
			image = new osg::Image; //FIXME: fare qualcosa se il nodo non viene caricato
		
		osg::ref_ptr<osg::Billboard> center = new osg::Billboard();
		center->setMode(osg::Billboard::POINT_ROT_EYE);
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> vert = new osg::Vec3Array;
		vert->push_back(osg::Vec3(1.0f,1.0f,0.0f));
		vert->push_back(osg::Vec3(0.0f,1.0f,0.0f));
		vert->push_back(osg::Vec3(0.0f,0.0f,0.0f));
		vert->push_back(osg::Vec3(1.0f,0.0f,0.0f));

		geom->setVertexArray( vert.get() );

		osg::ref_ptr<osg::Vec3Array> norm = new osg::Vec3Array;
		norm->push_back(osg::Vec3(0.0f,0.0f,1.0f));
		
		geom->setNormalArray( norm.get() );
		geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
		tcoords->push_back(osg::Vec2(0.0f,0.0f));
		tcoords->push_back(osg::Vec2(1.0f,0.0f));
		tcoords->push_back(osg::Vec2(1.0f,1.0f));
		tcoords->push_back(osg::Vec2(0.0f,1.0f));

		geom->setTexCoordArray(0, tcoords.get());

		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0, 4));

		if(image.valid())
		{
			osg::ref_ptr<osg::StateSet> stateset = geom->getOrCreateStateSet();
			osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
			texture->setImage( image.get() );
			stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
			geom->setStateSet( stateset.get() );
		}

		center->addDrawable( geom.get() );
		osg::Vec3 pos_finale;

		pos_finale.set( this->getPosition() );
		osg::Matrix bb_matrix;

		osg::Matrix bb_translate;

		bb_translate.makeTranslate( pos_finale );
			
		//PRIMA trasla, POI scala (studiare la geometria serviva...). Scala se serve...

		bb_matrix.preMult( bb_translate );
		//model_matrix.preMult( model_scale );
					
		bb_mt->setMatrix( bb_matrix );		

		bb_mt->addChild( center.get() );

		_added_models->addChild( bb_mt.get() );

		this->resetFlag();
	}
}