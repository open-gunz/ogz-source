@echo off
if not "%1" == "max" start /MAX cmd /c %0 max & exit/b

:: Save the current directory so that we can cd back to it at the end of the script.

set OLDCD=%CD%

:: Create and enter the build directory, and delete build\CMakeCache.txt if it exists for a clean build.

if not exist build (
	call mkdir build
)

if not exist build\win32 (
	call mkdir build\win32
)

call cd build/win32

:build

:: Generate the Visual Studio project files.

call cmake -G "Visual Studio 15 2017" -T "v141_xp" -Wno-dev -DBUILD_SHARED_LIBS=OFF -DZLIB_LIBRARY="C:\Program Files (x86)\zlib\lib\zlibstatic.lib" -DZLIB_INCLUDE_DIR="C:\Program Files (x86)\zlib\include" ..\..\src

if errorlevel 1 (
	echo cmake generation failed, exiting
	goto fail
)

:: Build in release mode.

call cmake --build . --config Release --target INSTALL

if errorlevel 1 (
	echo cmake build failed, exiting
	goto fail
)

echo.
echo.
echo Done!.
pause Press any key to rebuild. . . 
goto build

:fail
pause Press any key to rebuild. . . 
goto build