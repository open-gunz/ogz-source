@echo off

:: Save the current directory so that we can cd back to it at the end of the script.

set OLDCD=%CD%
if not exist build (
	call mkdir build
)

if exist build\win32 (
	echo build/win32 folder detected
	echo Redoing Project...
	
	call cd build/win32
	call cmake -G "Visual Studio 16 2019" -A Win32 -Wno-dev -DBUILD_SHARED_LIBS=OFF -DZLIB_LIBRARY="C:\Program Files (x86)\zlib\lib\zlibstatic.lib" -DZLIB_INCLUDE_DIR="C:\Program Files (x86)\zlib\include" ..\..\src
	
	echo Done.
	
	if errorlevel 1 (
		echo cmake generation failed, exiting
		goto fail
	)
	cd %OLDCD%
	pause
	exit /b
	
)

if not exist build\win32 (
	call mkdir build\win32
)

call cd build/win32

call cmake -G "Visual Studio 16 2019" -A Win32 -Wno-dev -DBUILD_SHARED_LIBS=OFF -DZLIB_LIBRARY="C:\Program Files (x86)\zlib\lib\zlibstatic.lib" -DZLIB_INCLUDE_DIR="C:\Program Files (x86)\zlib\include" ..\..\src

if errorlevel 1 (
	echo cmake generation failed, exiting
	goto fail
)

call cmake --build . --config Release --target INSTALL

if errorlevel 1 (
	echo cmake build failed, exiting
	goto fail
)

echo.
echo.
echo Done!
cd %OLDCD%
pause
exit /b

:fail
cd %OLDCD%
pause
exit /b 1