#ifndef __OSG4WEB_SHELLCOMMAND__
#define __OSG4WEB_SHELLCOMMAND__ 1

#include <string>

class ShellCommand
{
public:
	enum CommandType
		{
			SC_BAD_COMMAND = 0,
			SC_FORCE_COREADV_LOAD,
			SC_RELOAD_LCORE,
			SC_SHELL_VERSION
		};

	ShellCommand() : _command(SC_BAD_COMMAND) { };
	ShellCommand(CommandType ct, std::string arg) : _command(ct), _argument(arg) {};

protected:
	CommandType getCommand() { return _command; };
	void setCommand(CommandType cmd){ _command = cmd; };

	std::string getArgument() { return _argument; };
	void setArgument(std::string arg){ _argument = arg; };

private:
	CommandType _command;
	std::string _argument;
};



#endif //__OSG4WEB_SHELLCOMMAND__