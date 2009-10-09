

//TAKEN FROM OSG FileUtils

// currently this impl is for _all_ platforms, execpt as defined.
// the mac version will change soon to reflect the path scheme under osx, but
// for now, the above include is commented out, and the below code takes precedence.


#if defined(WIN32) && !defined(__CYGWIN__)
    #include <Io.h>
    #include <Windows.h>
    #include <Winbase.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <direct.h> // for _mkdir
    
    #define mkdir(x,y) _mkdir((x))
    #define stat64 _stati64

    // set up for windows so acts just like unix access().
    #define F_OK 4


#else // unix

#if defined( __APPLE__ )
    // I'm not sure how we would handle this in raw Darwin
    // without the AvailablilityMacros.
    #include <AvailabilityMacros.h>
    // 10.5 defines stat64 so we can't use this #define
    // By default, MAC_OS_X_VERSION_MAX_ALLOWED is set to the latest
    // system the headers know about. So I will use this as the control
    // variable. (MIN_ALLOWED is set low by default so it is 
    // unhelpful in this case.) 
    // Unfortunately, we can't use the label MAC_OS_X_VERSION_10_4
    // for older OS's like Jaguar, Panther since they are not defined,
    // so I am going to hardcode the number.
    #if (MAC_OS_X_VERSION_MAX_ALLOWED <= 1040)
        #define stat64 stat
    #endif
#elif defined(__CYGWIN__) || defined(__FreeBSD__)
    #define stat64 stat
#endif

    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

    // set up _S_ISDIR()
#if !defined(S_ISDIR)
#  if defined( _S_IFDIR) && !defined( __S_IFDIR)
#    define __S_IFDIR _S_IFDIR
#  endif
#  define S_ISDIR(mode)    (mode&__S_IFDIR)
#endif


#include <iostream>
#include <errno.h>
#include <stack>

#include <Utilities/FileUtils.h>

using namespace Utilities;

bool FileUtils::makeDirectory( const std::string &path )
{
    if (path.empty())
    {
		std::cout << "FileUtils::makeDirectory(): cannot create an empty directory" << std::endl;
        return false;
    }
    
    struct stat64 stbuf;
    if( stat64( path.c_str(), &stbuf ) == 0 )
    {
        if( S_ISDIR(stbuf.st_mode))
            return true;
        else
        {
			std::cout << "FileUtils::makeDirectory(): "  << 
                    path << " already exists and is not a directory!" << std::endl;
            return false;
        }
    }

    std::string dir = path;
    std::stack<std::string> paths;
    while( true )
    {
        if( dir.empty() )
            break;
 
        if( stat64( dir.c_str(), &stbuf ) < 0 )
        {
            switch( errno )
            {
                case ENOENT:
                case ENOTDIR:
                    paths.push( dir );
                    break;
 
                default:
					std::cout << "FileUtils::makeDirectory(): "  << strerror(errno) << std::endl;
                    return false;
            }
        }
		dir = FileUtils::getFilePath(std::string(dir));
    }

    while( !paths.empty() )
    {
        std::string dir = paths.top();
 
        #if defined(WIN32)
            //catch drive name
            if (dir.size() == 2 && dir.c_str()[1] == ':') {
                paths.pop();
                continue;
            }
        #endif

        if( mkdir( dir.c_str(), 0755 )< 0 )
        {
			std::cout << "FileUtils::makeDirectory(): "  << strerror(errno) << std::endl;
            return false;
        } 
        paths.pop();
    }
    return true;
}


bool FileUtils::makeDirectoryForFile( const std::string &path )
{
	return FileUtils::makeDirectory( FileUtils::getFilePath( path ));
}

void FileUtils::convertStringPathIntoFilePathList(const std::string& paths,FileUtils::FilePathList& filepath)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    char delimitor = ';';
#else
    char delimitor = ':';
#endif

    if (!paths.empty())
    {
        std::string::size_type start = 0;
        std::string::size_type end;
        while ((end = paths.find_first_of(delimitor,start))!=std::string::npos)
        {
            filepath.push_back(std::string(paths,start,end-start));
            start = end+1;
        }

        filepath.push_back(std::string(paths,start,std::string::npos));
    }
 
}

bool FileUtils::fileExists(const std::string& filename)
{
    return access( filename.c_str(), F_OK ) == 0;
}

bool FileUtils::fileDeletes(const std::string& filename)
{
	return ( remove( filename.c_str() ) != 0 );
}

FileUtils::FileType fileType(const std::string& filename)
{
    struct stat64 fileStat;
    if ( stat64(filename.c_str(), &fileStat) != 0 ) 
    {
		return FileUtils::FILE_NOT_FOUND;
    } // end if

    if ( fileStat.st_mode & S_IFDIR )
        return FileUtils::DIRECTORY;
    else if ( fileStat.st_mode & S_IFREG )
        return FileUtils::REGULAR_FILE;

    return FileUtils::FILE_NOT_FOUND;
}


std::string FileUtils::findFileInDirectory(const std::string& fileName,const std::string& dirName,FileUtils::CaseSensitivity caseSensitivity)
{
    bool needFollowingBackslash = false;
    bool needDirectoryName = true;
    FileUtils::DirectoryContents dc;

    if (dirName.empty())
    {
        dc = FileUtils::getDirectoryContents(".");
        needFollowingBackslash = false;
        needDirectoryName = false;
    }
    else if (dirName=="." || dirName=="./" || dirName==".\\")
    {
        dc = FileUtils::getDirectoryContents(".");
        needFollowingBackslash = false;
        needDirectoryName = false;
    }
    else
    {
        dc = FileUtils::getDirectoryContents(dirName);
        char lastChar = dirName[dirName.size()-1];
        if (lastChar=='/') needFollowingBackslash = false;
        else if (lastChar=='\\') needFollowingBackslash = false;
        else needFollowingBackslash = true;
        needDirectoryName = true;
    }

    for(FileUtils::DirectoryContents::iterator itr=dc.begin();
        itr!=dc.end();
        ++itr)
    {
        if ((caseSensitivity==CASE_INSENSITIVE && FileUtils::equalCaseInsensitive(fileName,*itr)) ||
            (fileName==*itr))
        {
            if (!needDirectoryName) return *itr;
            else if (needFollowingBackslash) return dirName+'/'+*itr;
            else return dirName+*itr;
        }
    }
    return "";
}

#if defined(WIN32) && !defined(__CYGWIN__)
    #include <io.h>
    #include <direct.h>

    FileUtils::DirectoryContents FileUtils::getDirectoryContents(const std::string& dirName)
    {
        FileUtils::DirectoryContents contents;

        WIN32_FIND_DATA data;
        HANDLE handle = FindFirstFile((dirName + "\\*").c_str(), &data);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                contents.push_back(data.cFileName);
            }
            while (FindNextFile(handle, &data) != 0);

            FindClose(handle);
        }
        return contents;
    }

#else

    #include <dirent.h>
    DirectoryContents getDirectoryContents(const std::string& dirName)
    {
        DirectoryContents contents;

        DIR *handle = opendir(dirName.c_str());
        if (handle)
        {
            dirent *rc;
            while((rc = readdir(handle))!=NULL)
            {
                contents.push_back(rc->d_name);
            }
            closedir(handle);
        }

        return contents;
    }

#endif // unix getDirectoryContexts


std::string FileUtils::getFilePath(const std::string& fileName)
{
    std::string::size_type slash1 = fileName.find_last_of('/');
    std::string::size_type slash2 = fileName.find_last_of('\\');
    if (slash1==std::string::npos) 
    {
        if (slash2==std::string::npos) return std::string();
        return std::string(fileName,0,slash2);
    }
    if (slash2==std::string::npos) return std::string(fileName,0,slash1);
    return std::string(fileName, 0, slash1>slash2 ?  slash1 : slash2);
}


std::string FileUtils::getSimpleFileName(const std::string& fileName)
{
    std::string::size_type slash1 = fileName.find_last_of('/');
    std::string::size_type slash2 = fileName.find_last_of('\\');
    if (slash1==std::string::npos) 
    {
        if (slash2==std::string::npos) return fileName;
        return std::string(fileName.begin()+slash2+1,fileName.end());
    }
    if (slash2==std::string::npos) return std::string(fileName.begin()+slash1+1,fileName.end());
    return std::string(fileName.begin()+(slash1>slash2?slash1:slash2)+1,fileName.end());
}


std::string FileUtils::getFileExtension(const std::string& fileName)
{
    std::string::size_type dot = fileName.find_last_of('.');
    if (dot==std::string::npos) return std::string("");
    return std::string(fileName.begin()+dot+1,fileName.end());
}

std::string FileUtils::convertFileNameToWindowsStyle(const std::string& fileName)
{
    std::string new_fileName(fileName);
    
    std::string::size_type slash = 0;
    while( (slash=new_fileName.find_first_of('/',slash)) != std::string::npos)
    {
        new_fileName[slash]='\\';
    }
    return new_fileName;
}

std::string FileUtils::convertFileNameToUnixStyle(const std::string& fileName)
{
    std::string new_fileName(fileName);
    
    std::string::size_type slash = 0;
    while( (slash=new_fileName.find_first_of('\\',slash)) != std::string::npos)
    {
        new_fileName[slash]='/';
    }

    return new_fileName;
}

bool FileUtils::isFileNameNativeStyle(const std::string& fileName)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    return fileName.find('/') == std::string::npos; // return true if no unix style slash exist
#else
    return fileName.find('\\') == std::string::npos; // return true if no windows style slash exist
#endif
}

std::string FileUtils::convertFileNameToNativeStyle(const std::string& fileName)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    return convertFileNameToWindowsStyle(fileName);
#else
    return convertFileNameToUnixStyle(fileName);
#endif
}



std::string FileUtils::getLowerCaseFileExtension(const std::string& filename)
{
    std::string ext = FileUtils::getFileExtension(filename);
    for(std::string::iterator itr=ext.begin();
        itr!=ext.end();
        ++itr)
    {
        *itr = tolower(*itr);
    }
    return ext;
}


// strip one level of extension from the filename.
std::string FileUtils::getNameLessExtension(const std::string& fileName)
{
    std::string::size_type dot = fileName.find_last_of('.');
    if (dot==std::string::npos) return fileName;
    return std::string(fileName.begin(),fileName.begin()+dot);
}


std::string FileUtils::getStrippedName(const std::string& fileName)
{
    std::string simpleName = getSimpleFileName(fileName);
    return getNameLessExtension( simpleName );
}


bool FileUtils::equalCaseInsensitive(const std::string& lhs,const std::string& rhs)
{
    if (lhs.size()!=rhs.size()) return false;
    std::string::const_iterator litr = lhs.begin();
    std::string::const_iterator ritr = rhs.begin();
    while (litr!=lhs.end())
    {
        if (tolower(*litr)!=tolower(*ritr)) return false;
        ++litr;
        ++ritr;
    }
    return true;
}

bool FileUtils::equalCaseInsensitive(const std::string& lhs,const char* rhs)
{
    if (rhs==NULL || lhs.size()!=strlen(rhs)) return false;
    std::string::const_iterator litr = lhs.begin();
    const char* cptr = rhs;
    while (litr!=lhs.end())
    {
        if (tolower(*litr)!=tolower(*cptr)) return false;
        ++litr;
        ++cptr;
    }
    return true;
}

bool FileUtils::containsServerAddress(const std::string& filename)
{
    // need to check for http://
    if (filename.size()<7) return false;
    if (filename.compare(0,7,"http://")==0) return true;
    return false;
}

std::string FileUtils::getServerAddress(const std::string& filename)
{
    if (filename.size()>=7 && filename.compare(0,7,"http://")==0)
    {
        std::string::size_type pos_slash = filename.find_first_of('/',7);
        if (pos_slash!=std::string::npos)
        {
            return filename.substr(7,pos_slash-7);
        }
        else
        {
            return filename.substr(7,std::string::npos);
        }
    }
    return "";
}

std::string FileUtils::getServerFileName(const std::string& filename)
{
    if (filename.size()>=7 && filename.compare(0,7,"http://")==0)
    {
        std::string::size_type pos_slash = filename.find_first_of('/',7);
        if (pos_slash!=std::string::npos)
        {
            return filename.substr(pos_slash+1,std::string::npos);
        }
        else
        {
            return "";
        }
    
    }
    return filename;
}

std::string FileUtils::concatPaths(const std::string& left, const std::string& right)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    const char delimiterNative  = '\\';
    const char delimiterForeign = '/';
#else
    const char delimiterNative  = '/';
    const char delimiterForeign = '\\';
#endif

    char lastChar = left[left.size() - 1];

    if(lastChar == delimiterNative)
    {
        return left + right;
    }
    else if(lastChar == delimiterForeign)
    {
        return left.substr(0, left.size() - 1) + delimiterNative + right;
    }
    else // lastChar != a delimiter
    {
        return left + delimiterNative + right;
    }
}

std::string FileUtils::getRealPath(const std::string& path)
{
#if defined(WIN32)  && !defined(__CYGWIN__)
    TCHAR retbuf[MAX_PATH + sizeof(TCHAR)];
    GetFullPathName(path.c_str(), sizeof(retbuf), retbuf, 0);
    return std::string(retbuf);
#else
    char resolved_path[PATH_MAX];
    char* result = realpath(path.c_str(), resolved_path);
    
    if (result) return std::string(resolved_path);
    else return path;
#endif 
}


