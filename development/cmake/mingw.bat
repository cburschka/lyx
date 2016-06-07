REM Run this script via double click.

REM Create a link and add  cmd.exe /k  in the preferences in front of the mingw.bat call to prevent closing the cmd window.

echo off

REM Install Qt from qt.io
set PATH=C:\Qt\Qt5.5.1\5.5\mingw492_32\bin;%PATH%
set PATH=C:\Qt\Qt5.5.1\Tools\mingw492_32\bin;%PATH%

set LYX_SOURCE=%~DP0..\..
set LYX_BUILD=%LYX_SOURCE%\..\compile-mingw

echo LyX source: %LYX_SOURCE%
echo LyX build : %LYX_BUILD%

REM Download http://sourceforge.net/projects/lyx/files/Win_installers/Dependencies/lyx20-deps-msvc2010-x86.zip
REM Extract on the same level as LyX sources
set GNUWIN32_DIR=%LYX_SOURCE%\..\lyx20-deps-msvc2010-x86\deps20

mkdir %LYX_BUILD%
rmdir /s/q %LYX_BUILD%
mkdir %LYX_BUILD%

cd %LYX_BUILD%
cmake %LYX_SOURCE% -G"MinGW Makefiles" -DLYX_PYTHON_EXECUTABLE=%GNUWIN32_DIR%\Python\python -DLYX_3RDPARTY_BUILD=1 -DLYX_USE_QT=QT5 -DLYX_MERGE_REBUILD=1 -DLYX_MERGE_FILES=1 -DLYX_NLS=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1 -DLYX_CONSOLE=OFF 

mingw32-make doc
mingw32-make translations
mingw32-make
mingw32-make install/strip

goto :eof
:eof
