/*

Settings for LyX installer

*/

;--------------------------------
;File locations

!define FILES_LICENSE "license.rtf"

!define FILES_LYX "..\..\..\..\build\installprefix"
!define FILES_ICONS "..\icons"
!define FILES_QT "..\..\..\..\..\..\local\qt-3"
!define FILES_DEPS "..\..\..\..\..\..\local"
!define FILES_MSYS "..\..\..\..\..\.."
!define FILES_MINGW "..\..\..\..\..\..\mingw"
!define FILES_ASPELL "C:\Aspell\bin"
!define FILES_ASPELLDATA "..\..\..\..\..\..\local\aspell"
!define FILES_DVIPOST "..\..\..\..\..\..\local\dvipost"
!define FILES_PYTHON "C:\Python24"
!define FILES_WINDOWS "C:\Windows\System32" ;For Python DLL
!define FILES_FONTS "..\bakoma"

;--------------------------------
;Locations of components to download

!define DOWNLOAD_LATEX "http://superb-west.dl.sourceforge.net/sourceforge/miktex/basic-miktex-2.4.2207.exe"
!define DOWNLOAD_IMAGEMAGICK "http://superb-west.dl.sourceforge.net/sourceforge/imagemagick/ImageMagick-6.2.8-0-Q16-windows-dll.exe"
!define DOWNLOAD_GHOSTSCRIPT "http://tug.ctan.org/tex-archive/nonfree/support/ghostscript/AFPL/gs853/gs853w32.exe"
!define DOWNLOAD_VIEWER "http://tug.ctan.org/tex-archive/nonfree/support/ghostscript/ghostgum/gsv48w32.exe"

;--------------------------------
;Download size (in KB)

!define SIZE_DOWNLOAD_LATEX 32470
!define SIZE_DOWNLOAD_IMAGEMAGICK 6582
!define SIZE_DOWNLOAD_GHOSTSCRIPT 9325
!define SIZE_DOWNLOAD_VIEWER 1459

;--------------------------------
;Approximations of space required for components (in KB)

!define SIZE_LATEX 133120
!define SIZE_IMAGEMAGICK 20480
!define SIZE_GHOSTSCRIPT 25600 
!define SIZE_VIEWER 4096

;--------------------------------
;Locations of setup files for components (for bundled setup)

!define INSTALL_LATEX "basic-miktex-2.4.2207.exe"
!define INSTALL_IMAGEMAGICK "ImageMagick-6.2.8-0-Q16-windows-dll.exe"
!define INSTALL_GHOSTSCRIPT "gs853w32.exe"
!define INSTALL_VIEWER "gsv48w32.exe"

;--------------------------------
;Names and version

!define APP_NAME "LyX"
!define /date APP_VERSION "1.4.2svn %Y%m%d"
!define APP_SERIES_NAME "1.4"
!define APP_SERIES_KEY "14"
!define APP_DIR_USERDATA "LyX1.4.x"
!define APP_INFO "${APP_NAME} - The Document Processor"

!define APP_RUN "bin\lyx.bat"
!define APP_EXE "bin\lyx.exe"

!define APP_REGKEY "Software\${APP_NAME}${APP_SERIES_KEY}"
!define APP_REGKEY_SETUP "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
!define APP_REGNAME_DOC "LyX.Document"

!define APP_EXT ".lyx"
!define APP_MIME_TYPE "application/lyx"

;--------------------------------
;Setup settings

!ifndef SETUPTYPE_BUNDLE
  !define /date SETUP_EXE "lyx-142svn-%Y%m%d.exe"
!else
  !define /date SETUP_EXE "lyx-142svn-%Y%m%d.exe-bundle.exe"
!endif

!define SETUP_DEFAULT_DIRECTORY "$PROGRAMFILES\${APP_NAME}${APP_SERIES_KEY}"
!define SETUP_ICON "${FILES_ICONS}\lyx_32x32.ico"
!define SETUP_HEADERIMAGE "graphics\header.bmp"
!define SETUP_WIZARDIMAGE "graphics\wizard.bmp"
!define SETUP_UNINSTALLER "Uninstall-${APP_NAME}.exe"
!define SETUP_UNINSTALLER_KEY "${APP_NAME}"

;--------------------------------
;Names of binaries to identify compontents

!define BIN_LATEX "latex.exe"
!define BIN_IMAGEMAGICK "convert.exe"
!define BIN_GHOSTSCRIPT "gswin32c.exe"

;--------------------------------
;Compession

SetCompressor /SOLID lzma
