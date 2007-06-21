/*

Settings for LyX installer

*/

#--------------------------------
#File locations

!define FILES_LICENSE "license.rtf"

!define FILES_ICONS "..\icons"
!define FILES_LAUNCHER "..\..\launcher"
!define FILES_PDFVIEW "..\..\pdfview"

!define FILES_MSVC "${FILES_WINDOWS}\WinSxS\x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.762_x-ww_6b128700"
!define FILES_NETPBM "${FILES_DEPS}\netpbm"
!define FILES_DTL "${FILES_DEPS}\dtl"
!define FILES_AIKSAURUS "${FILES_DEPS}\aiksaurus\"
!define FILES_DVIPOST "${FILES_DEPS}\dvipost"
!define FILES_DVIPOST_PKG "${FILES_DVIPOST}"
!define FILES_PDFTOOLS "${FILES_DEPS}\pdftools"

!define FILES_QT "${FILES_DEPS}\qt-4"
!define FILES_DVIPOSTDATA "${FILES_DEPS}\dvipost"
!define FILES_ASPELLDATA "${FILES_DEPS}\aspell"

!define FILES_NSISPLUGINS "${FILES_DEPS}\nsis"

#--------------------------------
#Locations of components to download

!define MIRROR_SF1 "http://superb-west.dl.sourceforge.net/sourceforge"
!define MIRROR_SF2 "http://mesh.dl.sourceforge.net/sourceforge"

!macro SourceForgeMirror ID FILENAME
  !define DOWNLOAD_${ID} "${MIRROR_SF1}/${FILENAME}"
  !define DOWNLOADALT_${ID} "${MIRROR_SF2}/${FILENAME}"
!macroend

!insertmacro SourceForgeMirror LATEX "miktex/basic-miktex-2.6.2704.exe"
!insertmacro SourceForgeMirror IMAGEMAGICK "imagemagick/ImageMagick-6.3.4-10-Q16-windows-dll.exe"
!insertmacro SourceForgeMirror GHOSTSCRIPT "ghostscript/gs857w32.exe"

!define DOWNLOAD_VIEWER "http://tug.ctan.org/tex-archive/nonfree/support/ghostscript/ghostgum/gsv48w32.exe"
!define DOWNLOADALT_VIEWER "http://ctan.basemirror.de/nonfree/support/ghostscript/ghostgum/gsv48w32.exe"

!define DOWNLOAD_ASPELLDICTS "ftp://ftp.lyx.org/pub/lyx/contrib/aspell6-windows"
!define DOWNLOADALT_ASPELLDICTS "http://www.lyx.org/~bpeng/aspell6-windows"

#--------------------------------
#Download size (in KB)

!define SIZE_DOWNLOAD_LATEX 53759
!define SIZE_DOWNLOAD_IMAGEMAGICK 7111
!define SIZE_DOWNLOAD_GHOSTSCRIPT 11916
!define SIZE_DOWNLOAD_VIEWER 1459

#--------------------------------
#Approximations of space required for components (in KB)

!define SIZE_LATEX 120000
!define SIZE_IMAGEMAGICK 18000
!define SIZE_GHOSTSCRIPT 30000
!define SIZE_VIEWER 4000

#--------------------------------
#Locations of setup files for components (for bundled setup)

!define INSTALL_LATEX "basic-miktex-2.6.2704.exe"
!define INSTALL_IMAGEMAGICK "ImageMagick-6.3.4-8-Q16-windows-dll.exe"
!define INSTALL_GHOSTSCRIPT "gs857w32.exe"
!define INSTALL_VIEWER "gsv48w32.exe"

#--------------------------------
#Names and version

!define APP_NAME "LyX"
!define /date APP_VERSION "1.5svn %Y%m%d"
!define APP_VERSION_NUMBER "1.5.0.0"
!define APP_SERIES_NAME "1.5"
!define APP_SERIES_KEY "15"
!define APP_DIR_USERDATA "LyX1.5.x"
!define APP_INFO "${APP_NAME} - The Document Processor"
!define APP_COPYRIGHT "LyX is Copyright © 1995 by Matthias Ettrich, 1995-2007 LyX Team"

!define APP_RUN "bin\lyx.exe"

!define APP_REGKEY "Software\${APP_NAME}${APP_SERIES_KEY}"
!define APP_REGKEY_SETUP "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
!define APP_REGKEY_SETTINGS "Software\${APP_NAME}${APP_SERIES_KEY}\Settings"

!define APP_REGNAME_DOC "LyX.Document"

!define APP_EXT ".lyx"
!define APP_MIME_TYPE "application/lyx"

#--------------------------------
#Setup settings

!ifndef SETUPTYPE_BUNDLE
  !define /date SETUP_EXE "lyx-15svn-%Y%m%d.exe"
!else
  !define /date SETUP_EXE "lyx-15svn-%Y%m%d-bundle.exe"
!endif

!define SETUP_DEFAULT_DIRECTORY "$PROGRAMFILES\${APP_NAME}${APP_SERIES_KEY}"
!define SETUP_ICON "${FILES_ICONS}\lyx_32x32.ico"
!define SETUP_HEADERIMAGE "graphics\header.bmp"
!define SETUP_WIZARDIMAGE "graphics\wizard.bmp"
!define SETUP_UNINSTALLER "Uninstall-${APP_NAME}.exe"
!define SETUP_UNINSTALLER_KEY "${APP_NAME}"

#--------------------------------
#Names of binaries to identify compontents

!define BIN_LATEX "tex.exe"
!define BIN_IMAGEMAGICK "convert.exe"
!define BIN_GHOSTSCRIPT "gswin32c.exe"

#--------------------------------
#Custom NSIS plug-ins

!addplugindir "${FILES_NSISPLUGINS}"
