#include <CommonCore/Manipulators/CameraPoints.h>


using namespace Manipulators;

CameraPoints::CameraPoints()
{
  
}

CameraPoints::~CameraPoints()
{
  EmptyCameraPoints();
}

void CameraPoints::AddCameraPoint(CameraData *Camera)
{
  CameraList.push_back(Camera);
}

void CameraPoints::DeleteCameraPoint(int index)
{
  free(CameraList.at(index));
  CameraList.erase(CameraList.begin() + index);
}

void CameraPoints::EmptyCameraPoints(void)
{
  int i, num;
  CameraData *Camera;

  num = CameraList.size();
  for (i = 0; i < num; i++)
  {
    Camera = (CameraData *)*CameraList.begin();
    if (Camera->point_name)
      free(Camera->point_name);
    free(Camera);
    CameraList.erase(CameraList.begin());
  }
}

CameraData *CameraPoints::GetCameraPoint(int index)
{
  return (CameraData *)CameraList.at(index);
}

int CameraPoints::GetNumPoints(void)
{
  return CameraList.size();
}