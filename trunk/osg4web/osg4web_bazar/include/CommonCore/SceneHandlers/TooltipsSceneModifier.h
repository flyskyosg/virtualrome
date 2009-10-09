#ifndef __OSG4WEB_TOOLTIPSCENEMODIFIER__
#define __OSG4WEB_TOOLTIPSCENEMODIFIER__ 1

#include <CommonCore/Scenehandlers/NodeParserHandler.h>

#include <osgText/Text>
#include <osg/MatrixTransform>
#include <osg/Switch>



namespace SceneHandlers
{

	/***********************************************************************
	 *
	 * TooltipsSceneModifier class
	 *
	 ***********************************************************************/
	class TooltipsSceneModifier : public NodeParserHandler::CommandStubb
	{
	public:
		//Costruttore 
		TooltipsSceneModifier(float initx, float inity);
		
		//Ritorna il tooltip HUD da attaccare allo ScenaGraph generale 
		osg::Node* createTooltipHUD();

		// Setta il nodo per ricevere l'evento tooltip, ritorna nodo da attaccare alla scena
		osg::Node* setTooltipsProperties(osg::Node* node, std::string descstr);

		//Setta il nodo di scena globale nel Tooltip
		void setMainSceneNode(osg::Node* node);

		//Ritorna la stringa di identificazione del Tooltip
		std::string getTooltipsCommand() { return this->getCommand(); }

		//Command Bridge ridefinizione
		bool commandBridge(std::string argument, osg::Node* node, osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea);

		//NodeMasks base functions
		osg::Node::NodeMask getAllowNodeMask() { return _allowmask; }
		osg::Node::NodeMask getNegateNodeMask() { return _negatemask; }

		void setAllowNodeMask(osg::Node::NodeMask  mask) { _allowmask = mask; }
		void setNegateNodeMask(osg::Node::NodeMask  mask) { _negatemask = mask; }

		void checkTooltipsTiming();

		//Ortho2D Settings
		void setOrtho2DInitResolution(float x, float y) { _xInitialResolution = x; _yInitialResolution = y; }

	protected:
		//Distruttore
		~TooltipsSceneModifier();
		//Inizializza lo SceneGraph Base di Tooltip
		void createTooltipSceneGraph();
		//Inizializza la Camera2D di rendering su schermo
		void createTooltipCamera2D();
		//Crea la geometria di Backgroud
		void createTooltipBackgroud(osg::BoundingBox bb);
		//Spegne il Tooltip
		void switchOffTooltip();
		//Crea e inizializza il Tooltip
		void switchOnTooltip(std::string argument);
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
		float _xCurrentResolution, _yCurrentResolution;
		//Risoluzione del Camera 2D allo startup
		float _xInitialResolution, _yInitialResolution;

		osg::Node::NodeMask _allowmask;
		osg::Node::NodeMask _negatemask;

		osg::Timer_t _startTime;

		double _maxTime;
		
	};

};



#endif //__OSG4WEB_TOOLTIPSCENEMODIFIER__