:: set here the path to Qt's bin folder and to the LyX-MSVC depend bin folder
:: (these are example paths)
PATH=D:\Qt\bin;D:\LyXSVN\lyx-devel\lyx-windows-deps-msvc2008\bin;%PATH%

:: change directory to the folder where the compile result should be stored
:: (this is an example path)
cd D:\LyXSVN\lyx-devel\compile-result
cmake ..\development\cmake -G"Visual Studio 9 2008" -Dnls=1 -DGNUWIN32_DIR=D:\LyXSVN\lyx-devel\lyx-windows-deps-msvc2008 -Dmerge=0

:: clean
start lyx.sln :: /clean Release

:: rebuild all generated files
::cmake ..\cmake -Dmerge_rebuild=1

:: build release version
::start lyx.sln /build Release

:: return to the cmake folder where this script was started from
:: (this is an example path)
cd D:\LyXSVN\lyx-devel\development\cmake

