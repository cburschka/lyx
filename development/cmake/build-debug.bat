:: fastest way to check if LyX compiles with cmake

:: be sure there is a solution
cmake ..\trunk\development\cmake -Dmerge=1

:: clean
devenv lyx.sln /clean Debug

:: rebuild all generated files
cmake ..\trunk\development\cmake -Dmerge_rebuild=1

:: build release version
devenv lyx.sln /build Debug

