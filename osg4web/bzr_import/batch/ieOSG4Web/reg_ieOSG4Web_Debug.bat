

echo copy
copy D:\Works\OSG-Devel\CMakeOSG4Web\Build\src\ieOSG4Web\debug\ieOSG4Web.dll D:\Works\OSG-Devel\CMakeOSG4Web\Debug\web\Firefox\plugins\ieOSG4Web.ocx
echo regsvr32
regsvr32 D:\Works\OSG-Devel\CMakeOSG4Web\Debug\web\Firefox\plugins\ieOSG4Web.ocx
