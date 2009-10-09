

#include <Utilities/StringUtils.h>

#include <sstream>

using namespace Utilities;


std::string StringUtils::numToString(int number)
{
	std::ostringstream convstream;
	convstream << number << std::flush;
	return convstream.str();
}

std::string StringUtils::numToString(float number)
{
	std::ostringstream convstream;
	convstream << number << std::flush;
	return convstream.str();
}

std::string StringUtils::numToString(double number)
{
	std::ostringstream convstream;
	convstream << number << std::flush;
	return convstream.str();
}

void StringUtils::splitLR(const std::string& str, std::string& left, std::string& right, const char& delimiter)
{
	std::string::size_type pos = str.find( delimiter );

	left = str.substr(0, pos);
	right = str.substr(pos + 1, str.size() -1);
}

void StringUtils::splitReverseLR(const std::string& str, std::string& left, std::string& right, const char& delimiter)
{
	std::string::size_type pos = str.find_last_of( delimiter );

	left = str.substr(0, pos);
	right = str.substr(pos + 1, str.size() -1);
}

void StringUtils::tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
	//TODO: da reimplementare con un parsing di stringhe avanzato
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) 
	{
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

//Cerca searchstr nella parte sinistra, divisa da delimiter, delle componenti del vettore e ritorno la parte destra se la trova;
std::string StringUtils::searchInVector(const std::vector<std::string>& base, std::string& searchstr, const char& delimiter)
{
	for(unsigned int i = 0; i < base.size(); i++)
	{
		std::string str = base.at(i);
		std::string left, right;

		StringUtils::splitLR(str, left, right, delimiter);

		if(left == searchstr)
			return right;
	}

	return std::string(); //stringa vuota
}