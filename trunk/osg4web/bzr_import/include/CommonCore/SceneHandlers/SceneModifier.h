#ifndef __OSG4WEB_SCENEMODIFIER__
#define __OSG4WEB_SCENEMODIFIER__ 1

#include <CommonCore/CommandSchedule.h>

#include <osg/Group>
#include <osg/Switch>
#include <osg/MatrixTransform>

#include <osgDB/ReaderWriter>


namespace SceneHandlers
{
	/***********************************************************************
	 *
	 * SceneModifier class
	 *
	 ***********************************************************************/
	class SceneModifier : public osg::Referenced, public CommandSchedule
	{
	public:
		enum SceneActions {
			CREATE_GROUP = 0,
			CREATE_SWITCH,
			CREATE_MATRIXTRANSFORM,
			DELETE_NODE,
			SWITCH_ALLCHILD,
			SWITCH_CHILD_BYNAME,
			SET_MATRIXIDENTITY,
			SET_MATRIX,
			SET_MATRIX_ROTATE,
			SET_MATRIX_SCALE,
			SET_MATRIX_TRANSLATE,
			GET_MATRIX,
			MOVE_NODE
		};

		//Costruttore
		SceneModifier();
		SceneModifier(osg::Group* grp);
		
		//Set/Get SceneData
		void setSceneData(osg::Group* data) { _SceneData = data; }
		osg::Group* getSceneData() { return _SceneData.get(); }

		//Set ReaderWriter Options
		void setReaderWriterOptions(osgDB::ReaderWriter::Options* opt) { _options = opt; }

		//Ridefinizioni del Gestore dei Comandi
		virtual std::string handleAction(std::string action, std::string argument);


	protected:
		//Distruttore
		~SceneModifier();

		//Inizializza i Comandi Accettati da JS
		void initCommandActions();

		//Base Scene Commands
		std::string createGroup(std::string action);
		std::string createSwitch(std::string action);
		std::string createMatrixTransform(std::string action);
		std::string deleteNode(std::string action);
		std::string switchAllChild(std::string action);
		std::string switchChildByName(std::string action);


		//Matrix Set Commands
		std::string setMatrixIdentityByName(std::string action);
		std::string setMatrixByName(std::string action);
		std::string setMatrixRotateByName(std::string action);
		std::string setMatrixScaleByName(std::string action);
		std::string setMatrixTranslateByName(std::string action);
		std::string getMatrixByName(std::string action);

		//Funzioni avanzate
		std::string moveNode(std::string action);

		
		//Funzioni di Supporto 

		//Attacca il nodo newnode alla scena tramite metodo action
		std::string addNodeToSceneParsingAction(osg::Node* newnode, std::string action);

		bool addNodeToParent(osg::Node* parent, osg::Node* child, std::string &retstr);

		bool removeNode(osg::Node* node, std::string &retstr);

		bool switchAllChildByNode(osg::Node* parent, std::string &retstr);
		bool switchNamedChildByNode(osg::Node* parent, std::string childname, std::string &retstr);

		//Funzioni di supporto per matrici di trasformazione

		bool setMatrixToMatTrans(osg::MatrixTransform* mt, osg::Matrix matrix);
		bool preMultMatrixToMatTrans(osg::MatrixTransform* mt, osg::Matrix matrix);
		bool postMultMatrixToMatTrans(osg::MatrixTransform* mt, osg::Matrix matrix);

	protected:
		//Main SceneNode
		osg::ref_ptr<osg::Group> _SceneData;

		//ReaderWriter Options
		osg::ref_ptr<osgDB::ReaderWriter::Options> _options;
	};

};

#endif //__OSG4WEB_SCENEMODIFIER__