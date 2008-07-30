REM -------- be shure that CD is not aliased by an env var --------
set CD=

REM -------- set DIR to the current directory --------
set DIR=%CD%
echo %CD%

REM -------- setup ENV --------
set PYTHONPATH=
set PYTHONPATH=%PYTHONPATH%;%DIR%\pyPackages\
REM set PYTHONPATH=%PYTHONPATH%;%DIR%\pyPackages\pythonOgre
set PYTHONPATH=%PYTHONPATH%;%DIR%\pyPackages\osg
set PYTHONPATH=%PYTHONPATH%;%DIR%\code\common

set PATH=
set PATH=%PATH%;%DIR%\bin
set PATH=%PATH%;%DIR%\python25
set PATH=%PATH%;%DIR%\python25\DLLs
set PATH=%PATH%;%DIR%\python25\Lib\site-packages\pywin32_system32
set PATH=%PATH%;%DIR%\pyPackages\osg\osgPlugins-2.5.0
REM set PATH=%PATH%;%DIR%\pyPackages\pythonOgre
REM set PATH=%PATH%;%DIR%\pyPackages\pythonOgre\plugins
set PATH=%PATH%;%DIR%\pyPackages\osg
set PATH=%PATH%;%DIR%\pyPackages\vtk

REM ---- DATADIR --- deve avere lo slash in fondo !! --------
set DATADIR=%DIR%\testData\

