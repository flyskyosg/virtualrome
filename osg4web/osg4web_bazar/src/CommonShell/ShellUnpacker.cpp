

#include <CommonShell/ShellUnpacker.h>

#include <Utilities/FileUtils.h>

#include <sstream>
#include <fstream>

ShellUnpacker::ShellUnpacker(std::string fname, std::string dest) : _fileName(fname),
	_destDirectory(dest),
	_unpackStubb(NULL),
	_unRARList(NULL),
	_stopunpack(false)
{

}

ShellUnpacker::~ShellUnpacker()
{
	freeCompressedFile();

	_unpackStubb = NULL;

	_destDirectory.clear();
	_fileName.clear();
}

void ShellUnpacker::setPackNameAndDestination(std::string fname, std::string dest)
{
	_destDirectory = dest;
	_fileName = fname;
}

void ShellUnpacker::freeCompressedFile()
{
	if(_unRARList)
	{
		urarlib_freelist(_unRARList);
		_unRARList = NULL;
	}
}

bool ShellUnpacker::checkRequirements()
{
	if(_fileName.empty() || _destDirectory.empty())
		return false;

	_fileName = Utilities::FileUtils::convertFileNameToNativeStyle(_fileName);
	_destDirectory = Utilities::FileUtils::convertFileNameToNativeStyle(_destDirectory);

	if(!Utilities::FileUtils::fileExists(_destDirectory))
		if(!Utilities::FileUtils::makeDirectory(_destDirectory))
			return false;

	return true;
}

//Return the file counter. -1 on error
int ShellUnpacker::openCompressedFile()
{
	if(!checkRequirements())
		return -2; //Return check requirements error;

	return urarlib_list((void*) _fileName.c_str(), (ArchiveList_struct*) &_unRARList);
}


int ShellUnpacker::unpackAll()
{
	int fileno = 0;
	int unpackcond = 0; //Condizione di decompressoine
	unsigned int cicleno = 0;

	freeCompressedFile();
	fileno = openCompressedFile();

	if(fileno < 0)
		return fileno;

	while(unpackcond == 0)
	{
		unpackcond = this->unpackFile();
		cicleno++;
		if(_unpackStubb)
			_unpackStubb->progressUnpackStubb(cicleno, fileno);

		if(_stopunpack)
		{
			unpackcond = -8; //STOP condition
			break;
		}
	}

	return unpackcond;
}

int ShellUnpacker::unpackFile()
{
	int ret = 1; //Return End
	char *data_ptr = NULL;
	unsigned long data_size = 0;  
	
	if(!_unRARList)
		return -3;

	if(urarlib_get(&data_ptr, &data_size, _unRARList->item.Name, (void*) _fileName.c_str(), NULL)) //FIXME: to finish password field
	{
		std::string newfile = Utilities::FileUtils::convertFileNameToNativeStyle(_destDirectory + "/" + std::string(_unRARList->item.Name));

		if(_unRARList->item.FileAttr == 16)
		{
			if(!Utilities::FileUtils::fileExists(newfile))
				if(!Utilities::FileUtils::makeDirectory(newfile))
					return -4; //Return writing directory error
		}
		else if(!this->writeFileToDisk(newfile, data_ptr))
		{
			if(data_ptr != NULL) 
				free(data_ptr);

			return -5; //Return Writing Error
		}
	
		ret = 0; //Return Continue
	} 
	else 
	{ 
		if(data_ptr != NULL) 
			free(data_ptr);

		return -1; //Return UnPacking Error
	}

	if(data_ptr != NULL) 
			free(data_ptr);
	
	_unRARList = (ArchiveList_struct*)_unRARList->next;
	
	return ret;
}

bool ShellUnpacker::writeFileToDisk(std::string filename, char* data)
{
	std::ofstream outfile(filename.c_str(), std::ofstream::binary);
	outfile << std::string(data);
	outfile.close();

	return true;
}
