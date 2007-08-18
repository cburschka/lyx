:: be sure there is a solution
cmake ..\trunk\development\cmake -Dmerge=1

:: clean
devenv lyx-qt4.sln /clean Release

:: rebuild all generated files
cmake ..\trunk\development\cmake -Dmerge_rebuild=1

:: build release version
devenv lyx-qt4.sln /build Release