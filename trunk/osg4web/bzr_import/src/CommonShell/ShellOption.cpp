
#include <CommonShell/ShellOption.h>

#include <string>

ShellOption::ShellOption()
{

}

ShellOption::~ShellOption()
{
	clearOption();
}

void ShellOption::clearOption()
{
	_shelloptionmap.clear();
}	

bool ShellOption::getShellOption(std::string name, std::string& value)
{
	value.clear(); //Resetto
	std::map<std::string, std::string>::iterator itr = _shelloptionmap.begin();
	for( ; itr != _shelloptionmap.end(); ++itr)
	{
		if( itr->first == name )
		{
			value = itr->second;
			return true;
		}
	}

	return false;
}

std::string ShellOption::getShellOption(std::string name)
{
	std::string temp;
	if(this->getShellOption(name, temp))
		return temp;
	else
		return std::string("");
}

void ShellOption::setShellOption(ShellOption::OptionPair opair)
{
	if(!findAndSet(opair))
		_shelloptionmap.insert(opair); 
}

bool ShellOption::findAndSet(ShellOption::OptionPair opair)
{
	std::string name = opair.first;

	std::map<std::string, std::string>::iterator itr = _shelloptionmap.begin();
	for( ; itr != _shelloptionmap.end(); ++itr)
	{
		if( itr->first == name )
		{
			itr->second = opair.second;
			return true;
		}
	}

	return false;
}