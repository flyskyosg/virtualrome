#ifndef __OSG4WEB_STRINGUTILS__
#define __OSG4WEB_STRINGUTILS__ 1

#include <string>
#include <vector>

#include <Utilities/Export.h>

namespace Utilities {


namespace StringUtils
{
	extern OSG4WEB_UTILITIES_EXPORT void splitLR(const std::string&, std::string&, std::string&, const char& delimiter = '=');

	extern OSG4WEB_UTILITIES_EXPORT void splitReverseLR(const std::string&, std::string&, std::string&, const char& delimiter = '=');

	extern OSG4WEB_UTILITIES_EXPORT void tokenize(const std::string&, std::vector<std::string>&, const std::string& delimiters = " \t\r\n;,");

	extern OSG4WEB_UTILITIES_EXPORT std::string searchInVector(const std::vector<std::string>&, std::string&, const char& delimiter = '=');

	extern OSG4WEB_UTILITIES_EXPORT std::string numToString(int number);

	extern OSG4WEB_UTILITIES_EXPORT std::string numToString(float number);

	extern OSG4WEB_UTILITIES_EXPORT std::string numToString(double number);
}


}




#endif //__OSG4WEB_STRINGUTILS__