#ifndef __OSG4WEB_SHELLCOMMAND__
#define __OSG4WEB_SHELLCOMMAND__ 1

#include <string>

class ShellCommand
{
public:
	enum CommandType
		{
			SC_BAD_COMMAND = 0,
			SC_LOADCORE_OPTIONS,
			SC_FORCE_COREADV_LOAD,
			SC_RELOAD_LCORE,
			SC_SET_COREADV,
			SC_SET_COREADV_INIT_OPTIONS,
			SC_SET_COREADV_START_OPTIONS,
			SC_SET_COREADV_SHA1HASH,
			SC_SET_COREADVDEP,
			SC_SET_COREADVDEP_SHA1HASH,
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