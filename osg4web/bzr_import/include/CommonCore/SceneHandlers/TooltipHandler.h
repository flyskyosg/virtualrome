#ifndef __OSG4WEB_TOOLTIPHANDLER__
#define __OSG4WEB_TOOLTIPHANDLER__ 1

#include <CommonCore/Scenehandlers/NodeParserHandler.h>

#include <osgText/Text>
#include <osg/MatrixTransform>
#include <osg/Switch>



namespace SceneHandlers
{

	/***********************************************************************
	 *
	 * TooltipHandler class
	 *
	 ***********************************************************************/
	class TooltipHandler : public NodeParserHandler
	{
	public:
		//Costruttore 
		TooltipHandler(osg::Node::NodeMask nm = 0xffffff);
		TooltipHandler(osg::Group* hudgrp, osg::Node::NodeMask nm = 0xffffff);
		
		//Setta il nodo di scena globale nel Tooltip
		void setMainSceneNode(osg::Node* node);

		//Metodo di gestione del FRAME event
		virtual void callmethodqueue(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		// Setta il nodo per ricevere l'evento tooltip, ritorna nodo da attaccare alla scena
		osg::Node* setTooltipsProperties(osg::Node* node, osg::Node::DescriptionList desclist);

		//Ritorna la stringa di identificazione del Tooltip
		std::string getTooltipsCommand() { return _tooltipcommandname; }
		
	protected:
		//Inizializza lo SceneGraph Base di Tooltip
		void createTooltipSceneGraph();
		//Inizializza la Camera2D di rendering su schermo
		void createTooltipCamera2D();
		//Crea la geometria di Backgroud
		void createTooltipBackgroud(osg::BoundingBox bb);
		//Spegne il Tooltip
		void switchOffTooltip();
		//Crea e inizializza il Tooltip
		void switchOnTooltip(NodeCommandList::NodeCommand ndc, const osgGA::GUIEventAdapter& ea);
		//Aggiorna la posizione del tooltip
		void moveTooltip(const osgGA::GUIEventAdapter& ea);
		
		osg::ref_ptr<osg::Geode> _geodend;
		osg::ref_ptr<osgText::Text> _textnd;
		osg::ref_ptr<osg::Geometry> _geombox;
		
		osg::ref_ptr<osg::MatrixTransform> _matrtrans;
		osg::ref_ptr<osg::Switch> _switchnd;
		
		osg::ref_ptr<osg::Group> _hudgrp;

		//Copia per check del nodo attivo
		osg::ref_ptr<osg::Node> _currentactivetooltip;

		//Risoluzione del Camera 2D
		float _xMaxResolution, _yMaxResolution;

	private:
		std::string _tooltipcommandname;
	};

};



#endif //__OSG4WEB_TOOLTIPHANDLER__