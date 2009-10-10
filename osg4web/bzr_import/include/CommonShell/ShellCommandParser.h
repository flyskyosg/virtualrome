#ifndef __OSG4WEB_SHELLCOMMANDPARSER__
#define __OSG4WEB_SHELLCOMMANDPARSER__ 1

#include <map>
#include <string>

#include <CommonShell/ShellCommand.h>

class ShellCommandParser : public ShellCommand
{
public:
	ShellCommandParser(std::string str);

	bool isValidString(){ return (getCommand() != ShellCommand::SC_BAD_COMMAND); };

	CommandType getShellCommand() { return this->getCommand(); };
	std::string getShellArgument() { return this->getArgument(); }

protected:
	void parseString();
	void parseCommand(std::string);

private:
	std::string _parsestring;

	std::map<std::string, ShellCommand::CommandType> _commandmap;
};

#endif //__OSG4WEB_SHELLCOMMANDPARSER__