#ifndef __CameraPoints_h__
#define __CameraPoints_h__

#include <vector>
using namespace std;


namespace Manipulators
{

	typedef struct {
	   double Camera_X,
			  Camera_Y,
			  Camera_Z,
			  Camera_Yaw,
			  Camera_Pitch,
			  Camera_Roll;
	   char   *point_name;
	} CameraData;

	class CameraPoints {
	   private:
		 vector<CameraData*> CameraList;

	   public:
		 CameraPoints();
		 ~CameraPoints();
		 void AddCameraPoint(CameraData *Camera);
		 void DeleteCameraPoint(int index);
		 CameraData *GetCameraPoint(int index);
		 int GetNumPoints(void);
		 void EmptyCameraPoints(void);
	};

};

#endif
