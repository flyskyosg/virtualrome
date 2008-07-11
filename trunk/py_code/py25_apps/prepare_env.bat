REM -------- be shure that CD is not aliased by an env var --------
set CD=

REM -------- set DIR to the current directory --------
set DIR=%CD%
echo %CD%

REM -------- setup ENV --------
rem set PYTHONPATH=%DIR%\python25\Lib\site-packages\
set PYTHONPATH=%DIR%\pyPackages\osg;%DIR%\code\common

set PATH=
set PATH=%PATH%;%DIR%\python25
set PATH=%PATH%;%DIR%\python25\DLLs
set PATH=%PATH%;%DIR%\python25\Lib\site-packages\pywin32_system32

set PATH=%PATH%;%DIR%\pyPackages\osg
set PATH=%PATH%;%DIR%\pyPackages\osg\osgPlugins-2.5.0
set PATH=%PATH%;%DIR%\pyPackages\vtk

REM ---- DATADIR --- deve avere lo slash in fondo !! --------
set DATADIR=%DIR%\testData\

