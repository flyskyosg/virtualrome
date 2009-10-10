#ifndef __OSG4WEBCORE_EXPORT__
#define __OSG4WEBCORE_EXPORT__ 1

// OSG DLL Export: from OSG code
#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))
    #pragma warning( disable : 4244 )
    #pragma warning( disable : 4251 )
    #pragma warning( disable : 4267 )
    #pragma warning( disable : 4275 )
    #pragma warning( disable : 4290 )
    #pragma warning( disable : 4786 )
    #pragma warning( disable : 4305 )
    #pragma warning( disable : 4996 )
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
	#  if defined( OSG4WEBCORE_LIBRARY_STATIC )
    #    define OSG4WEBCORE_EXPORT
    #  elif defined( OSG4WEBCORE_LIBRARY )
    #    define OSG4WEBCORE_EXPORT   __declspec(dllexport)
    #  else
    #    define OSG4WEBCORE_EXPORT   __declspec(dllimport)
    #  endif
#else
    #define OSG4WEBCORE_EXPORT
#endif 

#endif //__OSG4WEBCORE_EXPORT__



