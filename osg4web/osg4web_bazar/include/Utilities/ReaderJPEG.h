#ifndef OSG4WEB_UTILITIES_READERJPEG
#define OSG4WEB_UTILITIES_READERJPEG

#include <Utilities/Export.h>

namespace Utilities
{
	namespace Reader
	{
		namespace JPEG
		{
			extern OSG4WEB_UTILITIES_EXPORT unsigned char * LoadImageFromFile(std::string filename, int& width, int& height, int& numComponents);
		}
	}
}

#endif //OSG4WEB_UTILITIES_READERJPEG
