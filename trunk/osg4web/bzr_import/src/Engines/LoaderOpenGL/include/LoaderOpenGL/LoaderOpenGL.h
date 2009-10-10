#ifndef __OSG4WEB_LOADEROPENGL__
#define __OSG4WEB_LOADEROPENGL__ 1


#include <CoreOpenGL/CoreOpenGL.h>
#include <LoaderOpenGL/Defines.h>


/***********************************************************************
 *
 * LoaderOpenGL example class
 *
 ***********************************************************************/
class LoaderOpenGL : public CoreOpenGL
{
public:

	enum LoaderOpenGLActions
	{
		UNKNOWN_ACTION = 0,
		LOAD_MODEL,
		STATUSBAR_VALUE,
		STATUSBAR_COLOR,
		STATUSBAR_VISIBILITY,
		SETMESSAGE,
		SETMESSAGE_COLOR
	};

	//Costruttore/Distruttore
	LoaderOpenGL(std::string corename = OSG4WEB_LOADEROPENGL_NAME);
	~LoaderOpenGL();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_LOADEROPENGL_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_LOADEROPENGL_VERSION; };

	//Inizializza le Opzioni del core successivamente all'inizializzazione
	virtual void AddStartOptions(std::string str, bool erase = true) { }

	//Ridefinizioni del Gestore dei Comandi
	virtual std::string handleAction(std::string argument);

protected:



};

#endif //__OSG4WEB_LOADEROPENGL__




