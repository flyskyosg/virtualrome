#ifndef __OSG4WEBCORE_DEFINES__
#define __OSG4WEBCORE_DEFINES__

#if defined (WIN32)
#define WINDOWIDTYPE HWND //Win32 type
#define WINDOWHGLRC HGLRC
#define WINDOWHDC HDC
#else
#define WINDOWIDTYPE Window //X11 type
#define WINDOWHGLRC //TODO
#define WINDOWHDC //TODO
#endif


#define OSG4WEB_COREINTERFACE_VERSION	"1.0.0.0"

#define OSG4WEB_COREINTERFACE_LIBNAME	"osg4Web: Core Interface"

#define OSG4WEB_COREINTERFACE_NAME		"CoreInterface"


//Template per la creazione delle funzioni di interscambio
template<class cTemplate> cTemplate* newClass(){ return new cTemplate; }
template<class cTemplate> void deleteClass(cTemplate* delclass){ delete delclass; }

//Export DLL
#define CREATECLASSINSTANCE(CA,CB) extern "C" OSG4WEBCORE_EXPORT CA* createClassInstance() { return dynamic_cast<CA*>( newClass<CB>()) ; }
#define DELETECLASSINSTANCE(CA,CB) extern "C" OSG4WEBCORE_EXPORT void deleteClassInstance(CA* cb) { deleteClass<CB>( dynamic_cast<CB*>(cb) ); }



#endif //__OSG4WEBCORE_DEFINES__


