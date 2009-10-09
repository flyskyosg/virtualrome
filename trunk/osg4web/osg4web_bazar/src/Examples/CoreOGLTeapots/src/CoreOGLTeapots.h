#ifndef __OSG4WEB_COREOGLTEAPOTS__
#define __OSG4WEB_COREOGLTEAPOTS__ 1


#include <CoreOpenGL/CoreOpenGL.h>
#include <Defines.h>

using namespace CommonCore;


/***********************************************************************
 *
 * CoreOGLTeapots example class
 *
 ***********************************************************************/
class CoreOGLTeapots : public CoreOpenGL
{
public:
	//Costruttore
	CoreOGLTeapots(std::string corename = OSG4WEB_COREOGLTEAPOTS_NAME);
	//Distruttore
	~CoreOGLTeapots();
	
	//Return Class Name
	virtual const char* className() { return OSG4WEB_COREOGLTEAPOTS_LIBNAME; };
	//Return Class Version
	virtual const char* classVersion() { return OSG4WEB_COREOGLTEAPOTS_VERSION; };

protected:
	//OpenGL Initialization
	void initializeOpenGL();
	//OpenGL Window Resize
	bool windowResize(int windowX, int windowY, int windowWidth, int windowHeight);
	//OpenGL Rendering Scene
	bool renderImplementation();
	//Render Teapot
	void renderTeapot(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
	
	GLuint _teapotList;
};

#endif //__OSG4WEB_COREOGLTEAPOTS__




