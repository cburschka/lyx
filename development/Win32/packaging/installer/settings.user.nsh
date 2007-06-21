;LyX Windows Installer
;User Settings

;Uncomment the right settting for your build environment
;BUILD_MSVC = Microsoft Visual C++ 2005
;BUILD_MINGW = MinGW/MSYS

!define BUILD_MSVC
;!define BUILD_MINGW 

;Location of LyX files and dependencies

!ifdef BUILD_MSVC

  ;Settings for MSVC build

  !define FILES_LYX "..\..\..\..\build-msvc"
  !define FILES_DEPS "..\..\..\..\lyx-windows-deps-msvc"
  
!else

  ;Settings for MinGW build

  !define FILES_LYX "..\..\..\..\build\installprefix"
  !define FILES_DEPS "..\..\..\..\..\..\local"  
  !define FILES_MINGW "..\..\..\..\..\..\mingw"

!endif

;Location of Python 2.5

!define FILES_PYTHON "C:\Python25"
