#ifndef __OSG4WEB_LOADEROPENGL__
#define __OSG4WEB_LOADEROPENGL__ 1


#include <CoreOpenGL/CoreOpenGL.h>
#include <LoaderOpenGL/Defines.h>

#include <string>
#include <vector>
#include <time.h>


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
	//OpenGL Initialization
	virtual void initializeOpenGL();
	//OpenGL Window Resize
	virtual bool windowResize(int windowX, int windowY, int windowWidth, int windowHeight);
	//OpenGL Rendering Scene
	virtual bool renderImplementation();

	bool loadTexture(std::string filename, GLuint texturelist);

	bool loadTextureJPEG(std::string filename, GLuint texturelist);
	//bool loadTextureGIF(std::string filename, GLuint texturelist);

	bool handleShellMessages(std::string message);
	bool refreshStatusBarValue(std::string value);

	void createLines();
	void createLogoTextureQuad();
	void createTitleTextureQuad();
	void createMessageTextureQuad();

	void drawStatusBar();

	float _pbarrdownload, _pbarrunpack, _pbarrload;
	bool _showmessages;

	unsigned int _message_id;
	unsigned int _session;

	GLuint _line_list;
	
	GLuint _logo_list;
	GLuint _title_list;
	GLuint _message_list;

	GLuint _logo_texture_list;
	GLuint _title_texture_list;
	GLuint _message_texture_list;

	clock_t _saved_timer;
	unsigned int _image_tick;
};

#endif //__OSG4WEB_LOADEROPENGL__




