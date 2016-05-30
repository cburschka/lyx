REM Run this script via double click.

REM Create a link and add  cmd.exe /k  in the preferences in front of the mingw.bat call to prevent closing the cmd window.

::echo off

REM Install Qt from qt.io
set PATH=C:\Qt\Qt5.6.0-MSVC2015\5.6\msvc2015\bin;%PATH%

set LYX_SOURCE=%~DP0..\..
set LYX_BUILD=%LYX_SOURCE%\..\build-result-5-2015

echo LyX source: %LYX_SOURCE%
echo LyX build : %LYX_BUILD%


REM first remove an existing compilation to assure a clean version
mkdir %LYX_BUILD%
del /s/q %LYX_BUILD%\*
rmdir /s/q %LYX_BUILD%\LYX_INSTALLED

REM Download http://sourceforge.net/projects/lyx/files/Win_installers/Dependencies/lyx20-deps-msvc2010-x86.zip
REM Extract on the same level as LyX sources
set GNUWIN32_DIR=D:\LyXGit\Master\lyx-windows-deps-msvc2010

REM MSVC 2015 tools
call "C:\Programme (x86)\MSVC2015\VC\vcvarsall.bat" x86

cd %LYX_BUILD%
cmake %LYX_SOURCE% -G"NMake Makefiles" -DLYX_USE_QT=QT5 -DLYX_MERGE_FILES=1 -DLYX_NLS=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1 -DLYX_CONSOLE=OFF 

nmake doc
nmake translations
nmake
nmake install

goto :eof
:eof

REM go back to the dir where the script was called from
cd /D %CALLED_FROM%
