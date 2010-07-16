/*

declaration.nsh

Configuration of LyX installer

*/

SetCompressor /SOLID lzma

#--------------------------------
# Location of LyX files and dependencies

!ifdef FilesLyX
  !define FILES_LYX "${FilesLyX}"
!else
  !define FILES_LYX "..\..\..\..\..\lyx-1.6.8-install"
!endif

!ifdef FilesDeps
  !define FILES_DEPS "${FilesDeps}"
!else
  !define FILES_DEPS "..\..\..\..\deps"
!endif

!ifdef FilesBundle
  !define FILES_BUNDLE "${FilesBundle}"
!else
  !define FILES_BUNDLE "..\..\..\..\depsbundle"
!endif

!define FILES_QT "C:\Qt\4.6.3"

#--------------------------------
# File locations

!define FILES_LICENSE "license.rtf"

!define FILES_ICONS "..\icons"
!define FILES_LAUNCHER "..\..\launcher"
!define FILES_PDFVIEW "..\..\pdfview"

!define FILES_MSVC "${FILES_DEPS}\bin"
!define FILES_PYTHON "${FILES_DEPS}\python"
!define FILES_IMAGEMAGICK "${FILES_DEPS}\imagemagick"
!define FILES_GHOSTSCRIPT "${FILES_DEPS}\ghostscript"
!define FILES_NETPBM "${FILES_DEPS}\netpbm"
!define FILES_DTL "${FILES_DEPS}\dtl"
!define FILES_AIKSAURUS "${FILES_DEPS}\aiksaurus\"
!define FILES_DVIPOST "${FILES_DEPS}\dvipost"
!define FILES_DVIPOST_PKG "${FILES_DVIPOST}"
!define FILES_PDFTOOLS "${FILES_DEPS}\pdftools"
!define FILES_METAFILE2EPS "${FILES_DEPS}\metafile2eps"

!define FILES_ASPELLDATA "${FILES_DEPS}\aspell"

!define FILES_NSISPLUGINS "${FILES_DEPS}\nsis"

#--------------------------------
# Locations of components to download

# CTAN and SourceForge select a mirror automatically

!define DOWNLOAD_LATEX "http://www.ctan.org/get/systems/win32/miktex/setup/${SETUPFILE_LATEX}"
!define DOWNLOAD_ASPELLDICTS "ftp://ftp.lyx.org/pub/lyx/contrib/aspell6-windows"

#--------------------------------
# Locations of setup files for components (for bundled setup)

!define INSTALL_LATEX "${SETUPFILE_LATEX}"

#--------------------------------
# Names and version

!define APP_NAME "LyX"
!define APP_VERSION_NUMBER "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}.${APP_VERSION_BUILD}"
!define APP_SERIES_NAME "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}"
!define APP_SERIES_KEY "${APP_VERSION_MAJOR}${APP_VERSION_MINOR}"
!define APP_DIR "${APP_NAME}${APP_SERIES_KEY}"
!define APP_DIR_USERDATA "${APP_NAME}${APP_SERIES_KEY}"
!define APP_INFO "${APP_NAME} - The Document Processor"
!define APP_COPYRIGHT "LyX is Copyright © 1995 by Matthias Ettrich, 1995-2010 LyX Team"

!define APP_RUN "bin\LyXLauncher.exe"

!define APP_REGKEY "Software\${APP_NAME}${APP_SERIES_KEY}"
!define APP_REGKEY_SETUP "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
!define APP_REGKEY_SETTINGS "Software\${APP_NAME}${APP_SERIES_KEY}\Settings"

!define APP_REGNAME_DOC "LyX.Document"

!define APP_EXT ".lyx"
!define APP_MIME_TYPE "application/lyx"

#--------------------------------
# Setup settings

# Output file name can be configured using command line paramaters like
# /DExeFile=/path/to/installer

!ifndef SETUPTYPE
  !define SETUPTYPE STANDARD
!endif
 
!if ${SETUPTYPE} == STANDARD

  !define SETUP_EXE ${ExeFile}

!else if ${SETUPTYPE} == BUNDLE

  !define SETUP_EXE ${BundleExeFile}
  !define BUNDLESETUP_LATEX

!endif

!define SETUP_ICON "${FILES_ICONS}\lyx.ico"
!define SETUP_HEADERIMAGE "graphics\header.bmp"
!define SETUP_WIZARDIMAGE "graphics\wizard.bmp"
!define SETUP_UNINSTALLER "Uninstall-${APP_NAME}.exe"
!define SETUP_UNINSTALLER_KEY "${APP_NAME}"

#--------------------------------
# Names of binaries to identify compontents

!define BIN_LATEX "tex.exe"
!define BIN_BIBTEXEDITOR "JabRef.exe"

#--------------------------------
# Custom NSIS plug-ins

!addplugindir "${FILES_NSISPLUGINS}"
