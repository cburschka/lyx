REM Run this script via double click.

REM Create a link and add  cmd.exe /k  in the preferences in front of the mingw.bat call to prevent closing the cmd window.

::echo off

REM Install Qt from qt.io
set PATH=C:\Qt\Qt5-5-1-2010\5.5\msvc2010\bin;%PATH%

set LYX_SOURCE=%~DP0..\..

REM first remove an existing compilation to assure a clean version
RMDIR /S /Q %LYX_SOURCE%\build-result-5-2010

set LYX_BUILD=%LYX_SOURCE%\build-result-5-2010

echo LyX source: %LYX_SOURCE%
echo LyX build : %LYX_BUILD%

REM Download http://sourceforge.net/projects/lyx/files/Win_installers/Dependencies/lyx20-deps-msvc2010-x86.zip
REM Extract on the same level as LyX sources
set GNUWIN32_DIR=%LYX_SOURCE%\lyx-windows-deps-msvc2010

REM MSVC 2010 tools
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86


mkdir %LYX_BUILD%
rmdir /s/q %LYX_BUILD%
mkdir %LYX_BUILD%

cd %LYX_BUILD%
cmake %LYX_SOURCE% -G"NMake Makefiles" -DLYX_PYTHON_EXECUTABLE=%GNUWIN32_DIR%\Python\python -DLYX_3RDPARTY_BUILD=1 -DLYX_ENABLE_CXX11=ON -DLYX_USE_QT=QT5 -DLYX_MERGE_REBUILD=1 -DLYX_MERGE_FILES=1 -DLYX_NLS=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1 -DLYX_CONSOLE=OFF 

nmake doc
nmake translations
nmake
nmake install

goto :eof
:eof

REM go back to the dir where the script was called from
cd /D %CALLED_FROM%
