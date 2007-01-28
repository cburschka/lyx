;LyX Windows Installer
;User Settings, all these macros can be redefined from command line, 
;using options like /DFilesLyX="..\..\..\..\lyx-install"

;Location of LyX files and dependencies

!ifndef FilesLyX
!define FILES_LYX "..\..\..\..\build-msvc"
!endif

!ifndef FilesDeps
!define FILES_DEPS "..\..\..\..\lyx-windows-deps-msvc-qt4"
!endif

;Location of Windows installation

!ifndef FilesWindows
!define FILES_WINDOWS "C:\WINDOWS"
!endif

;Location of Python 2.5

!ifndef FilesPython
!define FILES_PYTHON "C:\Python25"
!endif
