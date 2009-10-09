#ifndef __OSG4WEB_SHELLOPTION__
#define __OSG4WEB_SHELLOPTION__ 1


#include <map>

class ShellOption
{
public:
	//Settaggio dei parametri di configurazione
	typedef std::pair<std::string, std::string> OptionPair;

	ShellOption();
	~ShellOption();

	void setShellOption(ShellOption::OptionPair opair);
	void setShellOption(std::string name, std::string value) { this->setShellOption(ShellOption::OptionPair(name, value)); }

	bool getShellOption(std::string name, std::string& value);
	std::string getShellOption(std::string name);

	void clearOption();

protected:
	bool findAndSet(ShellOption::OptionPair opair);

private:
	//String Option Map
	std::map<std::string,std::string> _shelloptionmap;

};



#endif //__OSG4WEB_SHELLOPTION__