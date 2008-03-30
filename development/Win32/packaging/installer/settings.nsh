/*

Settings for LyX installer

*/

SetCompressor /SOLID lzma

#--------------------------------
# Location of LyX files and dependencies

!ifdef FilesLyX
  !define FILES_LYX "${FilesLyX}"
!else
  !define FILES_LYX "..\..\..\..\build-msvc"
!endif

!ifdef FilesDeps
  !define FILES_DEPS "${FilesDeps}"
!else
  !define FILES_DEPS "..\..\..\..\lyx-windows-deps-msvc2008"
!endif

!ifdef FilesBundle
  !define FILES_BUNDLE "${FilesBundle}"
!else
  !define FILES_BUNDLE "..\..\..\..\lyx-windows-bundle-deps"
!endif

#--------------------------------
# Location of Python 2.5

!ifdef FilesPython
  !define FILES_PYTHON "${FilesPython}"
!else
  !define FILES_PYTHON "C:\Python25"
!endif

#--------------------------------
# File locations

!define FILES_LICENSE "license.rtf"

!define FILES_ICONS "..\icons"
!define FILES_LAUNCHER "..\..\launcher"
!define FILES_PDFVIEW "..\..\pdfview"

!define FILES_MSVC "$%SystemRoot%\WinSxS\x86_Microsoft.VC90.CRT_1fc8b3b9a1e18e3b_9.0.21022.8_x-ww_d08d0375"
!define FILES_NETPBM "${FILES_DEPS}\netpbm"
!define FILES_DTL "${FILES_DEPS}\dtl"
!define FILES_AIKSAURUS "${FILES_DEPS}\aiksaurus\"
!define FILES_PDFTOOLS "${FILES_DEPS}\pdftools"

!define FILES_QT "${FILES_DEPS}\qt-4"
!define FILES_ASPELLDATA "${FILES_DEPS}\aspell"

!define FILES_NSISPLUGINS "${FILES_DEPS}\nsis"

#--------------------------------
# Locations of components to download

!define MIRROR_SF1 "http://superb-west.dl.sourceforge.net/sourceforge"
!define MIRROR_SF2 "http://mesh.dl.sourceforge.net/sourceforge"

!macro SourceForgeMirror ID FILENAME
  !define DOWNLOAD_${ID} "${MIRROR_SF1}/${FILENAME}"
  !define DOWNLOADALT_${ID} "${MIRROR_SF2}/${FILENAME}"
!macroend

!insertmacro SourceForgeMirror LATEX "miktex/basic-miktex-2.7.2904.exe"
!insertmacro SourceForgeMirror IMAGEMAGICK "imagemagick/ImageMagick-6.3.7-8-Q16-windows-dll.exe"
!insertmacro SourceForgeMirror GHOSTSCRIPT "ghostscript/gs861w32.exe"

!define DOWNLOAD_ASPELLDICTS "ftp://ftp.lyx.org/pub/lyx/contrib/aspell6-windows"
!define DOWNLOADALT_ASPELLDICTS "http://www.lyx.org/~bpeng/aspell6-windows"

#--------------------------------
# Download size (in KB)

!define SIZE_DOWNLOAD_LATEX 78493
!define SIZE_DOWNLOAD_IMAGEMAGICK 7330
!define SIZE_DOWNLOAD_GHOSTSCRIPT 12469

#--------------------------------
# Approximations of space required for components (in KB)

!define SIZE_LATEX 225000
!define SIZE_IMAGEMAGICK 18700
!define SIZE_GHOSTSCRIPT 31500
#--------------------------------
# Locations of setup files for components (for bundled setup)

!define INSTALL_LATEX "basic-miktex-2.7.2904.exe"
!define INSTALL_IMAGEMAGICK "ImageMagick-6.3.7-8-Q16-windows-dll.exe"
!define INSTALL_GHOSTSCRIPT "gs861w32.exe"

#--------------------------------
# Names and version

!define APP_NAME "LyX"
!define /date APP_VERSION "1.6svn %Y%m%d"
!define APP_VERSION_NUMBER "1.6.0.0"
!define APP_SERIES_NAME "1.6"
!define APP_SERIES_KEY "16"
!define APP_DIR_USERDATA "LyX16"
!define APP_INFO "${APP_NAME} - The Document Processor"
!define APP_COPYRIGHT "LyX is Copyright © 1995 by Matthias Ettrich, 1995-2008 LyX Team"

!define APP_RUN "bin\lyx.exe"

!define APP_REGKEY "Software\${APP_NAME}${APP_SERIES_KEY}"
!define APP_REGKEY_SETUP "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
!define APP_REGKEY_SETTINGS "Software\${APP_NAME}${APP_SERIES_KEY}\Settings"

!define APP_REGNAME_DOC "LyX.Document"

!define APP_EXT ".lyx"
!define APP_MIME_TYPE "application/lyx"

#--------------------------------
# Setup settings

# Output file name can be configured using command line paramater
# /DExeFile=/path/to/installer or /DBundleExeFile=/path/to/installer if 
# SETUPTYPE_BUNDLE is defined.

!ifndef SETUPTYPE_BUNDLE
  !ifndef ExeFile
    !define /date ExeFile "LyX-16svn-%Y%m%d-Installer.exe"
  !endif
  !define SETUP_EXE "${ExeFile}"
!else
  !ifndef SETUPTYPE_BUNDLE
    !define /date BundleExeFile "LyX-16svn-%Y%m%d-Installer-Bundle.exe"
  !endif
  !define SETUP_EXE "${BundleExeFile}"  
!endif

!define SETUP_DEFAULT_DIRECTORY "$PROGRAMFILES\${APP_NAME}${APP_SERIES_KEY}"
!define SETUP_ICON "${FILES_ICONS}\lyx_32x32.ico"
!define SETUP_HEADERIMAGE "graphics\header.bmp"
!define SETUP_WIZARDIMAGE "graphics\wizard.bmp"
!define SETUP_UNINSTALLER "Uninstall-${APP_NAME}.exe"
!define SETUP_UNINSTALLER_KEY "${APP_NAME}"

#--------------------------------
# Names of binaries to identify compontents

!define BIN_LATEX "tex.exe"
!define BIN_IMAGEMAGICK "convert.exe"
!define BIN_GHOSTSCRIPT "gswin32c.exe"

#--------------------------------
# Custom NSIS plug-ins

!addplugindir "${FILES_NSISPLUGINS}"
