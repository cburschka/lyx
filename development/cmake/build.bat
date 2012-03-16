echo off

echo -------------------------------------------------------------------------------------
echo Usage build.bat devel/install/deploy STUDIO(optional)
echo     devel   - Builds Visual Studio project files for development on LyX
echo     install - Builds Visual Studio project files with all enabled for installation
echo     deploy  - Builds Makefiles and installs LyX
echo     STUDIO  - Used Visual Studio version, default is "Visual Studio 9 2008"
echo               use "Visual Studio 10" for Visual Studio 10
echo -------------------------------------------------------------------------------------
echo Be sure you've set qmake in PATH and set the variables:
echo     GNUWIN32_DIR
echo     LYX_SOURCE
echo     LXY_BUILD
echo Or edit this file.
echo -------------------------------------------------------------------------------------


if [%1]==[] (
	echo ERROR: no options.
    echo Exiting now.
	goto :eof
)

REM Add path to qmake here or set PATH correctly on your system.
set PATH=D:\Qt\bin;%PATH%

REM Edit pathes here or set the environment variables on you system.
set GNUWIN32_DIR=D:\LyXGit\2.0.x\lyx-windows-deps-msvc2010
set LYX_SOURCE=D:\LyXGit\2.0.x
set LYX_BUILD=D:\LyXGit\2.0.x\compile-result

if [%LYX_BUILD%]==[] (
	echo ERROR:  LYX_BUILD not set.
	echo Exiting now.
	goto :eof
)

if [%LYX_SOURCE%]==[] (
	echo ERROR:  LYX_SOURCE not set.
	echo Exiting now.
	goto :eof
)

if [%GNUWIN32_DIR%]==[] (
	echo ERROR:  GNUWIN32_DIR not set.
	echo Exiting now.
	goto :eof
)

echo LyX source: "%LYX_SOURCE%"
echo LyX build : "%LYX_BUILD%"
echo LyX deps  : "%GNUWIN32_DIR%"

set PATH=%GNUWIN32_DIR%\bin;%PATH%


if not exist %LYX_BUILD% (
	echo creating "%LYX_BUILD%"
	mkdir "%LYX_BUILD%"
	if not exist %LYX_BUILD% (
		echo Exiting script.
		goto :eof
	)
)
cd "%LYX_BUILD%"


REM Delete all files indirectory
::del /s/q *
del CMakeCache.txt

if [%2]==[] (
	set USED_STUDIO="Visual Studio 10"
) else (
	set USED_STUDIO=%2%
)


if "%1%" == "devel" (
	REM Build solution to develop LyX
	cmake %LYX_SOURCE%\development\cmake -G%USED_STUDIO% -DLYX_MERGE_FILES=0 -DLYX_NLS=1 -DLYX_RELEASE=0
	REM needed when running lyx from the debugger
	set LYX_DIR_20x=%LYX_SOURCE%\lib
	::start lyx.sln /build Debug
	msbuild lyx.sln /p:Configuration=Debug /t:LyX /t:tex2lyx
)

if "%1%" == "install" (
	REM Build solution to develop LyX
	cmake %LYX_SOURCE%\development\cmake -G%USED_STUDIO% -DLYX_MERGE_FILES=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1 -DLYX_CONSOLE=OFF
	REM needed when running lyx from the debugger
	set LYX_DIR_20x=
	::start lyx.sln /build Debug
	msbuild lyx.sln         /p:Configuration=Release /t:ALL_BUILD
	msbuild INSTALL.vcxproj /p:Configuration=Release 
)

if "%1%" == "deploy" (
	REM Build complete installed LyX
	cmake "%LYX_SOURCE%"\development\cmake -G"NMake Makefiles" -DLYX_MERGE_FILES=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1
	nmake
	nmake install
)

:: return to the cmake folder where this script was started from
cd %LYX_SOURCE%\development\cmake


:eof
