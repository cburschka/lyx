#LyX Windows Installer
#User Settings, all these macros can be redefined from command line, 
#using options like /DFilesLyX="..\..\..\..\lyx-install"

#Location of LyX files and dependencies

!ifdef FilesLyX
  !define FILES_LYX "${FilesLyX}"
!else
  !define FILES_LYX "..\..\..\..\build-msvc"
!endif

!ifdef FilesDeps
  !define FILES_DEPS "${FilesDeps}"
!else
  !define FILES_DEPS "..\..\..\..\lyx-windows-deps-msvc-qt4"
!endif

!ifdef FilesBundle
  !define FILES_BUNDLE "${FilesBundle}"
!else
  !define FILES_BUNDLE "..\..\..\..\lyx-windows-bundle-deps"
!endif

#Location of Python 2.5

!ifdef FilesPython
  !define FILES_PYTHON "${FilesPython}"
!else
  !define FILES_PYTHON "C:\Python25"
!endif
