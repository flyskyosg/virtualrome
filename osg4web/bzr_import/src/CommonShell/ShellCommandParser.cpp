
#include <string>

#include <CommonShell/ShellCommand.h>
#include <CommonShell/ShellCommandParser.h>

#include <Utilities/StringUtils.h>


ShellCommandParser::ShellCommandParser(std::string str) : ShellCommand(), 
	_parsestring(str)
{
	_commandmap["LOADCORE_OPTIONS"] = ShellCommand::SC_LOADCORE_OPTIONS;
	_commandmap["FORCE_COREADV_LOAD"] = ShellCommand::SC_FORCE_COREADV_LOAD;
	_commandmap["RELOAD_LCORE"] = ShellCommand::SC_RELOAD_LCORE;
	_commandmap["SET_COREADV"] = ShellCommand::SC_SET_COREADV;
	_commandmap["SET_COREADV_INIT_OPTIONS"] = ShellCommand::SC_SET_COREADV_INIT_OPTIONS;
	_commandmap["SET_COREADV_START_OPTIONS"] = ShellCommand::SC_SET_COREADV_START_OPTIONS;
	_commandmap["SET_COREADV_SHA1HASH"] = ShellCommand::SC_SET_COREADV_SHA1HASH;
	_commandmap["GET_SHELL_VERSION"] = ShellCommand::SC_SHELL_VERSION;
	_commandmap["SET_COREADVDEP"] = ShellCommand::SC_SET_COREADVDEP;
	_commandmap["SET_COREADVDEP_SHA1HASH"] = ShellCommand::SC_SET_COREADVDEP_SHA1HASH;

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
