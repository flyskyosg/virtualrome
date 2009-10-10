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

#include <CommonCore/CommandSchedule.h>


namespace SceneHandlers
{
	class PickEditHandler : public osgGA::GUIEventHandler, public CommandSchedule
	{

		public:

			enum Modes
			{
				VIEW = 0,		//sposto la camera
				ADD,			//modifico il modello attraverso il gizmo
				SELECT,			//seleziono il modello su cui applicare il gizmo
				PICK
			};
	        
	        
			PickEditHandler(osg::Group* nd): CommandSchedule("PICKEDIT"),
				_mode(VIEW),
				_model("http://localhost/osg4webtest/cow.ive"),
				_bbimagename("http://localhost/osg4webtest/bb/olmo2.png"),
				_add_node(false),
				_add_billboard(false),
				_model_billboard(true),
				_activeDragger(0),
				_dragger_name("TabBoxDragger"),
				_dragger_index(0),
				_cmdMgr(new osgManipulator::CommandManager),
				_ready_to_remove(false),
				_waitingCommit(false),
				_commit(false),
				_added_models(nd),
				_poseMatrixString("EMPTY")
				{
					//aggiungo i comandi per cambio modalità
					this->setCommandAction("VIEW");						//0
					this->setCommandAction("ADD");						//1
					this->setCommandAction("SELECT");					//2

					//comando per scelta modello da caricare con la ADD
					this->setCommandAction("CHANGE_MODEL");				//3

					//comando per scelta dragger da usare
					this->setCommandAction("CHANGE_DRAGGER");			//4

					//comando per scelta modello da caricare con la ADD
					this->setCommandAction("CHANGE_BILLBOARD");			//5

					//comando per staccare il gizmo
					this->setCommandAction("COMMIT");					//6

					//cattura ultima matrice di posa
					this->setCommandAction("GET_MATRIX");				//7
				
				}   

			~PickEditHandler() {}

			void setAddedModelGroup(osg::Group* nd) { _added_models = nd;  }
				
			bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);
			
			bool removeNode(osg::Node* child, osg::Node* parent);
			osg::Node* getChildNode();
			osg::Node* getParentNode();
			
			void resetCommitFlag();
			void resetRemoveFlag();
			bool commitStatus();
			bool removeStatus();
			void resetWaitingCommit();
			
			osgManipulator::CommandManager* getCmdMgr();
			
			std::string getDraggerName();
			osg::Node* getSelectionNode();
			
			void setActionView();
			
			void resetPointers();
			
			osg::Node* addDraggerToScene(osg::Node* nodo, osgManipulator::CommandManager* cmdMgr, const std::string& name);
			osgManipulator::Dragger* createDragger(const std::string& name);
			bool removeGizmo( osg::Node* model, osg::Node* parent, osg::Node* selection );
			
			osg::Vec3 getPosition();
			
			bool readyToAdd();
			bool readytoAddBillboard();
			
			void resetFlag();
			
			std::string getModel();

			std::string getBillBoardImageName();
					
			void storeHUDPointer(osg::Group* pHud);
			void defaultText();

			//permettono di cambiare modalità,modello e dragger 
			bool setMode(unsigned int mode);
			bool setModel(std::string model);
			bool setDragger(std::string dragger);
			bool setBillBoardImageName(std::string bimage);

			void handleSceneGraph(); //MODIF CARLO: handle della scena da preFrameUpdate

			//virtual void AddStartOptions(std::string str, bool erase = true);
			virtual std::string handleAction(std::string argument);

			//crea stringa a partire dalla matrice di posa
			void storeMatrixStream(osg::MatrixTransform* m);
			std::string getPoseMatrix();


	private:	
			
			unsigned int _mode;
			bool _add_node;				//flag per dire al viewer quando può attaccare
			bool _add_billboard;		//   "                 "              "        billboard   
			std::string _model;			//indirizzo modello da caricare
			std::string _bbimagename;	//nome immagine da caricare
			osg::Vec3 position;			//dove attaccare
			
			osgManipulator::Dragger*      _activeDragger;
			osgManipulator::PointerInfo   _pointer;

			bool _model_billboard;		//true quando attacco modelli, false altrimenti
			
			bool _ready_to_remove;		//ho in memoria nodo padre e figlio, posso staccarli
			bool _waitingCommit;		//se true, c'è un modello con gizmo attaccato, non posso crearne altri
			bool _commit;				//true quando il picker può togliere il gizmo

			osg::ref_ptr<osg::Node>      childNode;
			osg::ref_ptr<osg::Node>      parentNode;
			osgManipulator::Selection*   selection;

			std::string _dragger_name;
			int _dragger_index;
			osg::ref_ptr<osgManipulator::CommandManager> _cmdMgr;

			//memorizza il puntatore all'hud per cambiare il testo
			osg::ref_ptr<osgText::Text> textMode;
			osg::ref_ptr<osgText::Text> textDragger;
			osg::ref_ptr<osgText::Text> textModel;

			osg::ref_ptr<osg::Group>	_added_models;

			std::string					_poseMatrixString;		//per memorizzazione matrice di posa
			osg::ref_ptr<osgDB::ReaderWriter::Options> _options;

	};
};