#include <sstream>

#include <LoaderOpenGL/LoaderOpenGL.h>
#include <Utilities/FileUtils.h>
#include <Utilities/ReaderJPEG.h>


//LoaderOpenGL Costruttore
LoaderOpenGL::LoaderOpenGL(std::string corename) : CoreOpenGL(corename),
	_pbarrdownload(0.0), 
	_pbarrunpack(0.0),
	_pbarrload(0.0),
	_message_id(0),
	_session(0),
	_saved_timer(clock()),
	_image_tick(0),
	_showmessages(false)
{
	this->sendNotifyMessage("LoaderOpenGL -> Costructing LoaderOpenGL Core Instance.");

	//Ridefinisco il nome dello Schedule Command
	this->setCommandScheduleName("LOADER");

	//Default value
	this->setCommandAction("UNKNOWN_ACTION");
	this->setCommandAction("STATUSBAR_VALUE");
	this->setCommandAction("STATUSBAR_COLOR");
	this->setCommandAction("STATUSBAR_VISIBILITY");
	this->setCommandAction("SETMESSAGE");
	this->setCommandAction("SETMESSAGE_COLOR");

	this->addCommandSchedule((CommandSchedule*) this);
}

//LoaderOpenGL Distruttore
LoaderOpenGL::~LoaderOpenGL()
{
	this->sendNotifyMessage("~LoaderOpenGL -> Destructing Loader Core Instance.");
}


bool LoaderOpenGL::handleShellMessages(std::string message)
{
	if (message.empty())
	{
		_showmessages = false;
		return  true;
	}
	else if(message == "Core Found")
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 0;
	}
	else if (message ==  "Configuring Shell Options Failed" )
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 1;
	}
	else if(message ==  "Core Loading ..." )
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 2;
	}
	else if(message ==  "Loading Advanced Core Failed!" )
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 3;
	}
	else if(message ==  "Downloading Files..." )
	{
		_session = 1;
		_message_id = 4;
	}
	else if(message ==  "Downloading Failed!" )
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 5;
	}
	else if(message ==  "Checking Validity..." )
	{
		_session = 2;
		_pbarrdownload = _pbarrunpack  = 1.0;
		_message_id = 6;
	}
	else if(message ==  "Validity Control Failed!" )
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 7;
	}
	else if(message ==  "Unpacking Files..." )
	{
		_session = 2;
		_pbarrdownload = 1.0;
		_message_id = 8;
	}
	else if(message ==  "Unpacking Failed!" )
	{
		_session = 3;
		_pbarrdownload = _pbarrunpack = _pbarrload = 1.0f;
		_message_id = 9;
	}
	else
		return false;
	
	_showmessages = true;

	return true;
}

bool LoaderOpenGL::refreshStatusBarValue(std::string value) //FIXE: da testare
{
	bool ret = false;
	double newvalue;

	std::istringstream iss(value);
	ret = !(iss >> newvalue >> std::dec).fail();

	if(ret)
	{
		if(_session <= 1)
			_pbarrdownload = newvalue;
		else
			_pbarrunpack = newvalue;
	}

	return ret;
}

/** Ridefinizione della funzione di Gestione Comandi per CommandSchedule "this" */
std::string LoaderOpenGL::handleAction(std::string argument)
{
	this->sendNotifyMessage("handleAction -> serving command: " + argument);

	std::string retstr = "CORE_DONE";
	std::string lcommand, rcommand;

	this->splitActionCommand(argument, lcommand, rcommand);
	
	switch(this->getCommandActionIndex( lcommand ))
	{
	case STATUSBAR_VALUE: //STATUSBAR_VALUE
		if( !this->refreshStatusBarValue( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	case STATUSBAR_COLOR: //STATUSBAR_COLOR
	case STATUSBAR_VISIBILITY: //STATUSBAR_VISIBILITY
		break;
	case SETMESSAGE: //SETMESSAGE
		if( !this->handleShellMessages( rcommand ) )
			retstr = "CORE_FAILED";
		break;
	case SETMESSAGE_COLOR: //SETMESSAGE_COLOR
		break;
	default: //UNKNOWN_ACTION
		retstr = "UNKNOWN_ACTION";
		break;
	}

	return retstr;
}

void LoaderOpenGL::initializeOpenGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(OSG4WEB_BACKGROUND_COLOR, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glAlphaFunc(GL_GREATER,0.1f);
	glEnable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	//Creating Geometries
	this->createLines();
	this->createMessageTextureQuad();
	this->createLogoTextureQuad();
	this->createTitleTextureQuad();


	_logo_texture_list = glGenLists(OSG4WEB_LOADEROPENGL_NUM_LOGO_FRAMES);
	_title_texture_list = glGenLists(OSG4WEB_LOADEROPENGL_NUM_TITLE_FRAMES);
	_message_texture_list = glGenLists(OSG4WEB_LOADEROPENGL_NUM_MESSAGE_FRAMES);

	//Loading Logo Textures
	for(unsigned i = 0; i < OSG4WEB_LOADEROPENGL_NUM_LOGO_FRAMES; i++)
	{
		std::ostringstream number; //creates an ostringstream object
		number << i << std::flush;

		std::string filename = this->getInstallationDirectory() + std::string( "/" ) + OSG4WEB_LOADEROPENGL_IMAGES_SUBDIR +  std::string( "/" ) + std::string(OSG4WEB_LOADEROPENGL_LOGO_NAME_PREFIX) + number.str() + std::string(OSG4WEB_LOADEROPENGL_IMAGES_EXTENSIONS);
		this->loadTexture(filename, _logo_texture_list + i);
	}

	//Loading TITLE Textures
	for(unsigned i = 0; i < OSG4WEB_LOADEROPENGL_NUM_TITLE_FRAMES; i++)
	{
		std::ostringstream number; //creates an ostringstream object
		number << i << std::flush;

		std::string filename = this->getInstallationDirectory() + std::string( "/" ) + OSG4WEB_LOADEROPENGL_IMAGES_SUBDIR +  std::string( "/" ) + std::string(OSG4WEB_LOADEROPENGL_TITLE_NAME_PREFIX) + number.str() + std::string(OSG4WEB_LOADEROPENGL_IMAGES_EXTENSIONS);
		this->loadTexture(filename, _title_texture_list + i);
	}

	//Loading Messages Textures
	for(unsigned i = 0; i < OSG4WEB_LOADEROPENGL_NUM_MESSAGE_FRAMES; i++)
	{
		std::ostringstream number; //creates an ostringstream object
		number << i << std::flush;

		std::string filename = this->getInstallationDirectory() + std::string( "/" ) + OSG4WEB_LOADEROPENGL_IMAGES_SUBDIR +  std::string( "/" ) + std::string(OSG4WEB_LOADEROPENGL_MESSAGE_NAME_PREFIX) + number.str() + std::string(OSG4WEB_LOADEROPENGL_IMAGES_EXTENSIONS);
		this->loadTexture(filename, _message_texture_list + i);
	}
}

bool LoaderOpenGL::windowResize(int windowX, int windowY, int windowWidth, int windowHeight)
{
	glViewport(0, 0, (GLsizei) windowWidth, (GLsizei) windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat) windowWidth / (GLfloat) windowHeight, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return true;
}

bool LoaderOpenGL::renderImplementation()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	gluLookAt(0.0, 0.0, 10.0, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);

	glColor3f(OSG4WEB_BACKGROUND_COLOR);

	if(_showmessages)
	{
		GLuint id = (unsigned int) _message_id;
		glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glCallList(_message_texture_list + id); //id di messaggio
		glCallList(_message_list);
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	glColor3f(OSG4WEB_BACKGROUND_COLOR);

	clock_t currtime = clock();
	double seconds = (1 / 30.0f); //settato a 1/30 di sec
	if( currtime - _saved_timer > seconds  * CLOCKS_PER_SEC )
	{
		_saved_timer = currtime;
		_image_tick++;
	}
	
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glCallList(_title_texture_list + (_image_tick % OSG4WEB_LOADEROPENGL_NUM_TITLE_FRAMES));
	glCallList(_title_list);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glColor3f(OSG4WEB_BACKGROUND_COLOR);

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glCallList(_logo_texture_list + (_image_tick % OSG4WEB_LOADEROPENGL_NUM_LOGO_FRAMES) );
	glCallList(_logo_list);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glColor3f(OSG4WEB_BACKGROUND_COLOR);

	glPushMatrix();
	glCallList(_line_list);

	this->drawStatusBar();
	glPopMatrix();
	
	return true;
}

void LoaderOpenGL::drawStatusBar()
{
	float tx,ty,lm;
	float x1 = 0.0f, x2 = 0.07f, x3 = 0.22f, x4 = 0.17f;
	float y1 = 0.0f, y2 = 1.0f;

	ty = -1.5;
	lm = 1.0;

	if(_pbarrdownload != 0.0)
	{
		tx = -1.9;
		glBegin( GL_QUADS );
		glColor4f( OSG4WEB_COLORVEC_GREEN, 1.0f ); glVertex2f( x1 + tx, y1 * lm + ty);
		glColor4f( OSG4WEB_COLORVEC_GREEN, 0.3f ); glVertex2f( x2 + tx, (y2 * lm + ty ) * _pbarrdownload);
		glColor4f( OSG4WEB_COLORVEC_GREEN, 0.3f ); glVertex2f( x3 + tx, (y2  * lm + ty ) * _pbarrdownload);
		glColor4f( OSG4WEB_COLORVEC_GREEN, 1.0f ); glVertex2f( x4 + tx, y1 * lm + ty);
		glEnd();
	}

	if(_pbarrunpack != 0.0)
	{
		tx = -1.65;
		glBegin( GL_QUADS );
		glColor4f( OSG4WEB_COLORVEC_RED, 1.0f ); glVertex2f( x1 + tx, y1 * lm + ty);
		glColor4f( OSG4WEB_COLORVEC_RED, 0.3f ); glVertex2f( x2 + tx, (y2 * lm + ty ) * _pbarrunpack);
		glColor4f( OSG4WEB_COLORVEC_RED, 0.3f ); glVertex2f( x3 + tx, (y2 * lm + ty ) * _pbarrunpack);
		glColor4f( OSG4WEB_COLORVEC_RED, 1.0f ); glVertex2f( x4 + tx, y1 * lm + ty);
		glEnd();
	}

	if(_pbarrload != 0.0)
	{
		tx = -1.4;
		glBegin( GL_QUADS );
		glColor4f( OSG4WEB_COLORVEC_BLUE, 1.0f ); glVertex2f( x1 + tx, y1 * lm + ty);
		glColor4f( OSG4WEB_COLORVEC_BLUE, 0.3f ); glVertex2f( x2 + tx, (y2 * lm + ty ) * _pbarrload);
		glColor4f( OSG4WEB_COLORVEC_BLUE, 0.3f ); glVertex2f( x3 + tx, (y2 * lm + ty ) * _pbarrload);
		glColor4f( OSG4WEB_COLORVEC_BLUE, 1.0f ); glVertex2f( x4 + tx, y1 * lm + ty);
		glEnd();
	}
}

bool LoaderOpenGL::loadTexture(std::string filename, GLuint texturelist)
{
	std::string fname = Utilities::FileUtils::convertFileNameToNativeStyle( filename );
	std::string ext = Utilities::FileUtils::getFileExtension(filename);

	if(ext == "jpg")
	{
		if(this->loadTextureJPEG(fname, texturelist))
			return true;
		else
			return false;
	}
	
	return false;
}

bool LoaderOpenGL::loadTextureJPEG(std::string filename, GLuint texturelist)
{
	unsigned char *imageData = NULL;
	int width_ret;
	int height_ret;
	int numComponents_ret;

	imageData = Utilities::Reader::JPEG::LoadImageFromFile(filename, width_ret, height_ret, numComponents_ret);

	if (imageData==NULL) 
		return false;

	int s = width_ret;
	int t = height_ret;
	int r = 1;

	int internalFormat = numComponents_ret == 1 ? GL_LUMINANCE :
		numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
		numComponents_ret == 3 ? GL_RGB :
		numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

	unsigned int pixelFormat =
		numComponents_ret == 1 ? GL_LUMINANCE :
		numComponents_ret == 2 ? GL_LUMINANCE_ALPHA :
		numComponents_ret == 3 ? GL_RGB :
		numComponents_ret == 4 ? GL_RGBA : (GLenum)-1;

	unsigned int dataType = GL_UNSIGNED_BYTE;

	glNewList(texturelist, GL_COMPILE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width_ret, height_ret, internalFormat, GL_UNSIGNED_BYTE, imageData);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glEndList();
	
	return true;
}

void LoaderOpenGL::createLines()
{
	_line_list = glGenLists(1);
	glNewList(_line_list, GL_COMPILE);

	glColor3f( OSG4WEB_COLORVEC_GRAY ); 
	
	//Linea 1
	glBegin( GL_LINES );
	glVertex2f( -40.f, -1.6f );
	glVertex2f( -2.1f, -1.6f );
	glEnd();

	glBegin( GL_LINES );
	glVertex2f( -2.1f, -1.6f );
	glVertex2f( -1.9f, +1.6f );
	glEnd();

	glBegin( GL_LINES );
	glVertex2f( -1.9f, 1.6f );
	glVertex2f( 1.9f, 1.6f );
	glEnd();

	glBegin( GL_LINES );
	glVertex2f( 1.9f, 1.6f );
	glVertex2f( 1.8f, -0.05f );
	glEnd();
	
	glBegin( GL_LINES );
	glVertex2f( 1.8f, -0.05f );
	glVertex2f( 40.0f, -0.05f );
	glEnd();

	//Linea 2
	glBegin( GL_LINES );
	glVertex2f( -40.f, -1.5f );
	glVertex2f( -2.2f, -1.5f );
	glEnd();

	glBegin( GL_LINES );
	glVertex2f( -2.2f, -1.5f );
	glVertex2f( -2.0f, +1.7f );
	glEnd();

	glBegin( GL_LINES );
	glVertex2f( -2.0f, 1.7f );
	glVertex2f( 2.0f, 1.7f );
	glEnd();
	
	glBegin( GL_LINES );
	glVertex2f( 2.0f, 1.7f );
	glVertex2f( 1.9f, 0.05f );
	glEnd();

	glBegin( GL_LINES );
	glVertex2f( 1.9f, 0.05f );
	glVertex2f( 40.0f, 0.05f );
	glEnd();
	
	glEndList();
}

void LoaderOpenGL::createMessageTextureQuad()
{
	_message_list = glGenLists(1);
	glNewList(_message_list, GL_COMPILE);

	glBegin( GL_QUADS );
	glTexCoord2f(0.0f, 0.0f); glVertex2f( -1.0, -1.5);
	glTexCoord2f(0.0f, 1.0f); glVertex2f( -1.0, -0.5);
	glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.7, -0.5);
	glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.7, -1.5);
	glEnd();

	glEndList();
}

void LoaderOpenGL::createLogoTextureQuad()
{
	_logo_list = glGenLists(1);
	glNewList(_logo_list, GL_COMPILE);

	glBegin( GL_QUADS );
	glTexCoord2f(0.0f, 0.0f); glVertex2f( -1.8, -0.2);
	glTexCoord2f(0.0f, 1.0f); glVertex2f( -1.8, 1.2);
	glTexCoord2f(1.0f, 1.0f); glVertex2f( -0.4, 1.2);
	glTexCoord2f(1.0f, 0.0f); glVertex2f( -0.4, -0.2);
	glEnd();

	glEndList();
}

void LoaderOpenGL::createTitleTextureQuad()
{
	_title_list = glGenLists(1);
	glNewList(_title_list, GL_COMPILE);

	glColor4f( 1.0, 1.0, 1.0, 1.0 ); 
	glBegin( GL_QUADS );
	glTexCoord2f(0.0f, 0.0f); glVertex2f( -0.4, -0.4);
	glTexCoord2f(0.0f, 1.0f); glVertex2f( -0.4, 1.4);
	glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.7, 1.4);
	glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.7, -0.4);
	glEnd();

	glEndList();
}