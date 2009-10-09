#ifndef __OSG4WEB_FILEUTILS_INCLUDED__
#define __OSG4WEB_FILEUTILS_INCLUDED__ 1

#include <vector>
#include <deque>
#include <string>

#include <Utilities/Export.h>


namespace Utilities
{

namespace FileUtils
{

	enum CaseSensitivity
	{
		CASE_SENSITIVE,
		CASE_INSENSITIVE
	};

	enum FileType
	{
		FILE_NOT_FOUND,
		REGULAR_FILE,
		DIRECTORY
	};

	/** list of directories to search through which searching for files. */
	typedef std::deque<std::string> FilePathList;

	/** simple list of names to represent a directory's contents. */
	typedef std::vector<std::string> DirectoryContents;

	extern OSG4WEB_UTILITIES_EXPORT bool makeDirectory( const std::string &path );

	// Make a new directory for a given file.
	extern OSG4WEB_UTILITIES_EXPORT bool makeDirectoryForFile( const std::string &filePath );

	extern OSG4WEB_UTILITIES_EXPORT void convertStringPathIntoFilePathList(const std::string& paths,FilePathList& filepath);
    
	/** return true if a file exisits. */
	extern OSG4WEB_UTILITIES_EXPORT bool fileExists(const std::string& filename);

	/** return true if remove file success. */
	extern OSG4WEB_UTILITIES_EXPORT bool fileDeletes(const std::string& filename);

	/** return type of file. */
	extern OSG4WEB_UTILITIES_EXPORT FileType fileType(const std::string& filename);

	extern OSG4WEB_UTILITIES_EXPORT std::string findFileInDirectory(const std::string& fileName,const std::string& dirName,CaseSensitivity caseSensitivity=CASE_SENSITIVE);

	/** return the contents of a directory.
	  * returns an empty array on any error.*/
	extern OSG4WEB_UTILITIES_EXPORT DirectoryContents getDirectoryContents(const std::string& dirName);

	extern OSG4WEB_UTILITIES_EXPORT std::string getFilePath(const std::string& filename);
	extern OSG4WEB_UTILITIES_EXPORT std::string getFileExtension(const std::string& filename);
	extern OSG4WEB_UTILITIES_EXPORT std::string getLowerCaseFileExtension(const std::string& filename);
	extern OSG4WEB_UTILITIES_EXPORT std::string getSimpleFileName(const std::string& fileName);
	extern OSG4WEB_UTILITIES_EXPORT std::string getNameLessExtension(const std::string& fileName);
	extern OSG4WEB_UTILITIES_EXPORT std::string getStrippedName(const std::string& fileName);

	extern OSG4WEB_UTILITIES_EXPORT std::string convertFileNameToWindowsStyle(const std::string& fileName);
	extern OSG4WEB_UTILITIES_EXPORT std::string convertFileNameToUnixStyle(const std::string& fileName);

	extern OSG4WEB_UTILITIES_EXPORT bool isFileNameNativeStyle(const std::string& fileName);
	extern OSG4WEB_UTILITIES_EXPORT std::string convertFileNameToNativeStyle(const std::string& fileName);

	extern OSG4WEB_UTILITIES_EXPORT bool equalCaseInsensitive(const std::string& lhs,const std::string& rhs);
	extern OSG4WEB_UTILITIES_EXPORT bool equalCaseInsensitive(const std::string& lhs,const char* rhs);

	extern OSG4WEB_UTILITIES_EXPORT bool containsServerAddress(const std::string& filename);
	extern OSG4WEB_UTILITIES_EXPORT std::string getServerAddress(const std::string& filename);
	extern OSG4WEB_UTILITIES_EXPORT std::string getServerFileName(const std::string& filename);

	/** Concatenates two paths */
	extern OSG4WEB_UTILITIES_EXPORT std::string concatPaths(const std::string& left, const std::string& right);

	/** Removes .. and . dirs in a path */
	extern OSG4WEB_UTILITIES_EXPORT std::string getRealPath(const std::string& path);
}


}


#endif //__OSG4WEB_FILEUTILS__