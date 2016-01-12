echo off

set PATH=C:\Qt\Qt5.6.0\5.6\mingw492_32\bin;%PATH%
set PATH=C:\Qt\Qt5.6.0\Tools\mingw492_32\bin;%PATH%

set LYX_SOURCE=%~DP0..\..
set LYX_BUILD=%LYX_SOURCE%\..\compile-mingw

echo LyX source: %LYX_SOURCE%
echo LyX build : %LYX_BUILD%

set GNUWIN32_DIR=%LYX_SOURCE%\..\msvc2010-deps
set PATH="%GNUWIN32_DIR%\deps20\Python";%PATH%

mkdir %LYX_BUILD%
rmdir /s/q %LYX_BUILD%
mkdir %LYX_BUILD%

cd %LYX_BUILD%
cmake %LYX_SOURCE% -GNinja -DLYX_3RDPARTY_BUILD=ON -DLYX_ENABLE_CXX11=ON -DLYX_USE_QT=QT5 -DLYX_MERGE_REBUILD=1 -DLYX_MERGE_FILES=1 -DLYX_NLS=1 -DLYX_INSTALL=1 -DLYX_RELEASE=1 -DLYX_CONSOLE=OFF 

ninja
ninja install

cd ..

goto :eof
:eof
