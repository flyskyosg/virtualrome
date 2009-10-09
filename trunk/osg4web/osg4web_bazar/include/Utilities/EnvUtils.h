#ifndef __OSG4WEB_ENVUTILS__
#define __OSG4WEB_ENVUTILS__ 1

#include <Utilities/Export.h>

#include <string>

namespace Utilities
{

namespace EnvUtils
{
	class OSG4WEB_UTILITIES_EXPORT Environment
	{
	public:
		Environment();

		std::string getPath();
		std::string getTempDirectory();
		std::string getAppDirectory();

		bool addDirectoryToPath(std::string directory, std::string adder = std::string());

		bool restoreOriginalPath();

		std::string beginAddDirectoryToDirList(std::string dirlist, std::string add);
		std::string endAddDirectoryToDirList(std::string dirlist, std::string add);

		std::string composeDirectory(std::string directory, std::string adder);
		std::string composeVariable(std::string varname, std::string argument);

		std::string getEnvironmentVariable(std::string var);
		bool setEnvironmentVariable(std::string var, std::string arg);
		
	private:
		std::string _initpath;
		std::string _path;
		std::string _temp;
		std::string _app;
	};
}

}



#endif //__OSG4WEB_ENVUTILS__