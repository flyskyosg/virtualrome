@echo off

REM Setting Environment
set PATH=%MOZILLA_DEV_TOOLS_DLL%;%PATH%

echo Working Directory: %CD%

REM Gen Header
echo running -- xpidl -w -v -m header -I"..\..\..\var\dependancies\gecko-sdk\idl" "..\..\..\idl\npOSG4Web\%1.idl"
xpidl -w -v -m header -I"..\..\..\var\dependancies\gecko-sdk\idl" "..\..\..\idl\npOSG4Web\%1.idl"

IF EXIST ..\..\..\include\npOSG4Web\%1.h (
	echo "Found old %1.h in ..\..\..\include: checking differences"
	REM Confronto il vecchio ed il nuovo file
	fc /b %1.h ..\..\..\include\npOSG4Web\%1.h | find "FC: nessuna differenza riscontrata" > exittrue.txt

	REM delete 0 file
	for /F %%A in ("exittrue.txt") do If %%~zA equ 0 del exittrue.txt
		

	IF EXIST exittrue.txt (
		del /f  exittrue.txt %1.h
		echo %1: not changed! Skipping IDL creation
		GOTO end
	) ELSE (
		echo "Checking differences english version"
		REM Confronto il vecchio ed il nuovo file - INGLESE	
		fc /b %1.h ..\..\..\include\npOSG4Web\%1.h | find "FC: no differences encountered" > exittrue.txt

		REM delete 0 file
		for /F %%A in ("exittrue.txt") do If %%~zA equ 0 del exittrue.txt
	
		IF EXIST exittrue.txt (
			del /f  exittrue.txt %1.h
			echo %1: not changed! Skipping IDL creation
			GOTO end
		)

	)

)


echo moving-- %1.h ..\..\..\include\npOSG4Web\%1.h
move %1.h ..\..\..\include\npOSG4Web\%1.h

REM Gen TypeLib
echo running -- xpidl -w -v -m typelib -I"..\..\..\var\dependancies\gecko-sdk\idl" "..\..\..\idl\npOSG4Web\%1.idl"
xpidl -w -v -m typelib -I"..\..\..\var\dependancies\gecko-sdk\idl" "..\..\..\idl\npOSG4Web\%1.idl"

REM Collapse in single XPT
echo running -- xpt_link "..\..\..\bin\nposg4web.xpt" "%1.xpt"
xpt_link "..\..\..\bin\npOSG4Web.xpt" "%1.xpt"

echo Deleting intermediary file
del /f %1.xpt

:end

