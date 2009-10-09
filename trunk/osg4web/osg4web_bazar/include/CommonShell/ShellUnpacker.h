#ifndef __OSG4WEB_SHELLUNPACKER__
#define __OSG4WEB_SHELLUNPACKER__ 1

#include <CommonShell/Defines.h>

#include <string>

#include <unrarlib.h>


class ShellUnpacker
{
public:
	class ProgressUnpackStubb
	{
	public:
		virtual int progressUnpackStubb(unsigned int cicleno, int filenumber) { return -1; }
	};

	ShellUnpacker(std::string fname, std::string dest);
	~ShellUnpacker();

	void setProgressUnpackStubb(ProgressUnpackStubb* upcallback) { _unpackStubb = upcallback; }
	void setPackNameAndDestination(std::string fname, std::string dest);

	int openCompressedFile();
	int unpackFile();

	int unpackAll();

	void stopUnpacking() { _stopunpack = true; }

protected:
	
	void freeCompressedFile();

	bool checkRequirements();

	bool writeFileToDisk(std::string filename, char* data);

private:
	ArchiveList_struct* _unRARList;
	ProgressUnpackStubb* _unpackStubb;

	std::string _fileName;
	std::string _destDirectory;

	bool _stopunpack;
};

#endif //__OSG4WEB_SHELLUNPACKER__