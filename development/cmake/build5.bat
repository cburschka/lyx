echo off

echo -------------------------------------------------------------------------------------
echo -
echo Usage build.bat devel/install/deploy STUDIO(optional)
echo     devel   - Builds Visual Studio project files for development on LyX
echo     install - Builds Visual Studio project files with all enabled for installation
echo     STUDIO  - Used Visual Studio version, default is "Visual Studio 10"
echo               use "Visual Studio 9 2008" for Visual Studio 9
echo
echo -------------------------------------------------------------------------------------
echo -
echo Call this script from a build directory or set variables (or edit this file)
echo     LYX_SOURCE
echo     LXY_BUILD
echo
echo Be sure you've set qmake in PATH.
echo -
echo GNUWin32 will be downloaded if you don't set the variable GNUWIN32_DIR
echo -------------------------------------------------------------------------------------
echo -
echo -

if [%1]==[] (
	echo ERROR: no options.
    echo Exiting now.
	goto :eof
)

REM Uncomment the following line if you want to compile in parallel.
REM This uses N cl.exe processes, where N is the number of cores.
set CL=/MP

REM Save path
set CALLED_FROM=%CD%

REM Add path to qmake here or set PATH correctly on your system.
set PATH=C:\Qt\Qt5-5-1-2013-32bit\5.5\msvc2013\bin;%PATH%

REM Edit pathes here or set the environment variables on you system.
set GNUWIN32_DIR=D:\LyXGit\Master\lyx-windows-deps-msvc2013
set LYX_SOURCE=D:\LyXGit\Master\
set LYX_BUILD=D:\LyXGit\Master\compile-2013

if [%LYX_SOURCE%]==[] (
	set LYX_SOURCE=%~DP0\..\..
)
echo LyX source: "%LYX_SOURCE%"

if [%LYX_BUILD%]==[] (
	set LYX_BUILD=%CD%
)
echo LyX build : "%LYX_BUILD%"

if [%GNUWIN32_DIR%]==[] (
	echo GNUWIN32_DIR not set.
	echo Downloading win32 deps.
	set DEPENDENCIES_DOWNLOAD="-DLYX_DEPENDENCIES_DOWNLOAD=1"
) else set PATH="%GNUWIN32_DIR%\bin";%PATH%

if not exist %LYX_BUILD% (
	echo creating "%LYX_BUILD%"
	mkdir "%LYX_BUILD%"
	if not exist %LYX_BUILD% (
		echo Exiting script.
		goto :eof
	)
)

cd "%LYX_BUILD%"

REM start with a new cmake run
::del CMakeCache.txt

if [%2]==[] (
	set USED_STUDIO="Visual Studio 12 2013"
) else (
	set USED_STUDIO=%2%
)


if "%1%" == "devel" (
	REM Build solution to develop LyX
	cmake %LYX_SOURCE% -GNinja -G%USED_STUDIO% -DLYX_ENABLE_CXX11=ON -DLYX_MERGE_FILES=0 -DLYX_NLS=1 -DLYX_INSTALL=0 -DLYX_RELEASE=0 -DLYX_CONSOLE=FORCE %DEPENDENCIES_DOWNLOAD%
	msbuild lyx.sln /p:Configuration=Debug /t:ALL_BUILD
)

if "%1%" == "install" (
	REM Build solution to develop LyX
	REM set -DLYX_MERGE_REBUILD and -DLYX_MERGE_FILES to 1 for a version released with an installer
	cmake %LYX_SOURCE% -G%USED_STUDIO% -DLYX_ENABLE_CXX11=ON -DLYX_MERGE_REBUILD=1 -DLYX_MERGE_FILES=1 -DLYX_NLS=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1 -DLYX_CONSOLE=OFF %DEPENDENCIES_DOWNLOAD% 
	msbuild lyx.sln         /p:Configuration=Release /t:ALL_BUILD
	msbuild INSTALL.vcxproj /p:Configuration=Release
)

REM go back to the dir where the script was called from
cd /D %CALLED_FROM%

:eof
