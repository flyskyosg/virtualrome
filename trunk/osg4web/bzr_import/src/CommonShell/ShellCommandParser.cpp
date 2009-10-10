
#include <string>

#include <CommonShell/ShellCommand.h>
#include <CommonShell/ShellCommandParser.h>

#include <Utilities/StringUtils.h>


ShellCommandParser::ShellCommandParser(std::string str) : ShellCommand(), 
	_parsestring(str)
{
	_commandmap["FORCE_COREADV_LOAD"] = ShellCommand::SC_FORCE_COREADV_LOAD;
	_commandmap["RELOAD_LCORE"] = ShellCommand::SC_RELOAD_LCORE;
	_commandmap["GET_SHELL_VERSION"] = ShellCommand::SC_SHELL_VERSION;

	this->parseString();
}

void ShellCommandParser::parseString()
{
	std::string left, right;

	Utilities::StringUtils::splitLR(_parsestring, left, right);
	
	this->parseCommand(left);

	this->setArgument(right);
}

void ShellCommandParser::parseCommand(std::string left)
{
	std::map<std::string, ShellCommand::CommandType>::iterator itr = _commandmap.find(left); 

	if(_commandmap.end() != itr)
		this->setCommand(itr->second);
	else
		this->setCommand(ShellCommand::SC_BAD_COMMAND);	
}
