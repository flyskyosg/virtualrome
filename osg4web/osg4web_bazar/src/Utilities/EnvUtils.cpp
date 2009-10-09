

#include <Utilities/EnvUtils.h>
#include <Utilities/FileUtils.h>

#include <iostream>


using namespace Utilities;

using namespace EnvUtils;

Environment::Environment()
{
	std::string envpath(getEnvironmentVariable( "PATH" ));

#if defined(WIN32)
	std::string envtmp(getEnvironmentVariable( "TEMP" ));
	std::string envapp(getEnvironmentVariable( "APPDATA" ));

	if(envtmp.empty())
		envtmp = getEnvironmentVariable( "TMP" );
#else
	//TODO:
	std::string envtmp(getEnvironmentVariable( "tmp" ));
	std::string envapp(getEnvironmentVariable( "APPDATA" )); 
#endif

	if( envtmp.empty() )
	{
		std::cout << "Environment(): " << "Temp directory not found. Please specify a TEMP folder" << std::endl;
		return;
	}

	_temp = envtmp;
	_path = envpath;
	_initpath = envpath;
	_app = envapp;
}

std::string Environment::getPath()
{
	return _path;
}

std::string Environment::getTempDirectory()
{
	return _temp;
}

std::string Environment::getAppDirectory()
{
	return _app;
}

bool Environment::restoreOriginalPath()
{
	_path = _initpath;

	return setEnvironmentVariable("PATH",_initpath);
}

bool Environment::addDirectoryToPath(std::string directory, std::string adder)
{
	std::string ndir, newpath;

	if(!adder.empty())
		ndir = composeDirectory(directory, adder);
	else
		ndir = directory;

	if(!_path.empty())
		newpath = beginAddDirectoryToDirList(_path, ndir);
	else
		newpath = ndir;

	_path = newpath;

	return setEnvironmentVariable("PATH",_path);
}

std::string Environment::composeVariable(std::string varname, std::string argument)
{
	std::string retstr(varname);

	retstr.append("=");
	retstr.append(argument);
	
	return retstr;
}
std::string Environment::composeDirectory(std::string directory, std::string adder)
{
	std::string retstr;
	std::string dirsep("/");

	retstr.append(directory);
	retstr.append(dirsep);
	retstr.append(adder);

	return FileUtils::convertFileNameToNativeStyle(retstr);
}

std::string Environment::endAddDirectoryToDirList(std::string dirlist, std::string add)
{
	std::string retstr(dirlist);
	std::string separator, dirsep;
	
#if defined(WIN32)
	separator = ";";
#else
	separator = ":";
#endif

	if(!dirlist.empty())
		retstr += separator;

	retstr += add;
		
	return retstr;
}

std::string Environment::beginAddDirectoryToDirList(std::string dirlist, std::string add)
{
	std::string retstr(add);
	std::string separator, dirsep;
	
#if defined(WIN32)
	separator = ";";
#else
	separator = ":";
#endif

	if(!dirlist.empty())
		retstr += separator;

	retstr += dirlist;
		
	return retstr;
}

std::string Environment::getEnvironmentVariable(std::string var)
{
	return std::string( getenv( var.c_str() ) );
}
		
bool Environment::setEnvironmentVariable(std::string var, std::string arg)
{
	if( var.empty() )
		return false;

	return (putenv((char*) this->composeVariable(var,arg).c_str()) >= 0);
}

