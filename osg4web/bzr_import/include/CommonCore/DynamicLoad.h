#ifndef __OSG4WEB_DYNAMICLOAD__
#define __OSG4WEB_DYNAMICLOAD__ 1


//The dlopen calls were not adding to OS X until 10.3 
#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if !defined(MAC_OS_X_VERSION_10_3) || (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_3)
#define APPLE_PRE_10_3
#endif
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
#include <mach-o/dyld.h>
#else // all other unix
#include <unistd.h>
#ifdef __hpux__
// Although HP-UX has dlopen() it is broken! We therefore need to stick
// to shl_load()/shl_unload()/shl_findsym()
#include <dl.h>
#include <errno.h>
#else
#include <dlfcn.h>
#endif
#endif

#include <string>

class DynamicLoad
{
public:

	typedef void*   HANDLE;
	typedef void*   PROC_ADDRESS;

	~DynamicLoad()
	{
		if (_handle)
		{
#if defined(WIN32) && !defined(__CYGWIN__)
		    FreeLibrary((HMODULE)_handle);
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
		    NSUnLinkModule(static_cast<NSModule>(_handle), FALSE);
#elif defined(__hpux__)
		    // fortunately, shl_t is a pointer
		    shl_unload (static_cast<shl_t>(_handle));
#else // other unix
		    dlclose(_handle);
#endif    
		}
	};

	/** return name of library stripped of path.*/
	const std::string& getName() const     { return _name; }

	/** return name of library including full path to it.*/
	const std::string& getFullName() const { return _fullName; }

	/** return name of library including full path to it.*/
	static const std::string getLibraryExtension() 
	{ 
#if defined(WIN32) && !defined(__CYGWIN__)
		return std::string(".dll");
#else
		return std::string(); //FIXME: controllare
#endif
	}

	/** return handle to .dso/.dll dynamic library itself.*/
	HANDLE             getHandle() const   { return _handle; }

	/** return address of function located in library.*/
	PROC_ADDRESS       getProcAddress(const std::string& procName)
	{
		if (_handle==NULL) return NULL;
#if defined(WIN32) && !defined(__CYGWIN__)
		return 
			(DynamicLoad::PROC_ADDRESS)GetProcAddress( (HMODULE)_handle, procName.c_str() );
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
		std::string temp("_");
		NSSymbol symbol;
		temp += procName;   // Mac OS X prepends an underscore on function names
		symbol = NSLookupSymbolInModule(static_cast<NSModule>(_handle), temp.c_str());
		return NSAddressOfSymbol(symbol);
#elif defined(__hpux__)
		void* result = NULL;
		if (shl_findsym (reinterpret_cast<shl_t*>(&_handle), procName.c_str(), TYPE_PROCEDURE, result) == 0)
		{
		    return result;
		}
		else
		{
		    notify(WARN) << "DynamicLibrary::failed looking up " << procName << std::endl;
		    notify(WARN) << "DynamicLibrary::error " << strerror(errno) << std::endl;
		    return NULL;
		}
#else // other unix
		void* sym = dlsym( _handle,  procName.c_str() );
		if (!sym) {
		    notify(WARN) << "DynamicLibrary::failed looking up " << procName << std::endl;
		    notify(WARN) << "DynamicLibrary::error " << dlerror() << std::endl;
		}
		return sym;
#endif
		return NULL;
	};

	/** returns a pointer to a DynamicLibrary object on successfully
	* opening of library returns NULL on failure.
	*/
	static DynamicLoad* loadLibrary(const std::string& libraryName, std::string& errmsg)
	{
		HANDLE handle = NULL;

		handle = getLibraryHandle( libraryName, errmsg ); 
        
		if (handle) return new DynamicLoad(libraryName,handle);

		return NULL;
	};

protected:
	/** get handle to library file */
	static HANDLE getLibraryHandle( const std::string& libraryName, std::string& errmsg)
	{
		HANDLE handle = NULL;

#if defined(WIN32) && !defined(__CYGWIN__)
		handle = LoadLibrary( (libraryName + std::string(".dll")).c_str() );
		
		if(!handle)
		{
			LPTSTR winstr;
			if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, (LPTSTR)&winstr, 0, NULL) == 0)
					errmsg = "Unknown Error";
			else
					errmsg = std::string( (LPTSTR) winstr );
		}
     
#elif defined(__APPLE__) && defined(APPLE_PRE_10_3)
		NSObjectFileImage image;
		// NSModule os_handle = NULL;
		if (NSCreateObjectFileImageFromFile(libraryName.c_str(), &image) == NSObjectFileImageSuccess) {
		    // os_handle = NSLinkModule(image, libraryName.c_str(), TRUE);
		    handle = NSLinkModule(image, libraryName.c_str(), TRUE);
		    NSDestroyObjectFileImage(image);
		}
#elif defined(__hpux__)
		// BIND_FIRST is neccessary for some reason
		handle = shl_load ( libraryName.c_str(), BIND_DEFERRED|BIND_FIRST|BIND_VERBOSE, 0);
		return handle;
#else // other unix

		//TODO: aggiungere "lib" prima del nome
		// dlopen will not work with files in the current directory unless
		// they are prefaced with './'  (DB - Nov 5, 2003).
		std::string localLibraryName;
		if( libraryName == osgDB::getSimpleFileName( libraryName ) )
		    localLibraryName = "./" + libraryName;
		else
		    localLibraryName = libraryName;
		handle = dlopen( localLibraryName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if( handle == NULL )
		    notify(INFO) << "DynamicLibrary::getLibraryHandle( "<< libraryName << ") - dlopen(): " << dlerror() << std::endl;
#endif
		return handle;
	};

	/** disallow default constructor.*/
	DynamicLoad() {}
        
	/** Disallow public construction so that users have to go
	 * through loadLibrary() above which returns NULL on 
	 * failure, a valid DynamicLibrary object on success.
	 */
	DynamicLoad(const std::string& name, HANDLE handle)
	{
		_name = name;
		_handle = handle;
	};

private:
	HANDLE          _handle;
	std::string     _name;
	std::string     _fullName;
};


#endif //__OSG4WEB_DYNAMICLOAD__