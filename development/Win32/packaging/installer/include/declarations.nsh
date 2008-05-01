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
# File locations

!define FILES_LICENSE "license.rtf"

!define FILES_ICONS "..\icons"
!define FILES_LAUNCHER "..\..\launcher"
!define FILES_PDFVIEW "..\..\pdfview"

!define FILES_MSVC "$%SystemRoot%\WinSxS\x86_Microsoft.VC90.CRT_1fc8b3b9a1e18e3b_9.0.21022.8_x-ww_d08d0375"
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
!define FILES_PSPRINTER "${FILES_DEPS}\metafile2eps\PSPrinter"

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

!insertmacro SourceForgeMirror LATEX "miktex/${SETUPFILE_LATEX}"
!insertmacro SourceForgeMirror IMAGEMAGICK "imagemagick/${SETUPFILE_IMAGEMAGICK}"
!insertmacro SourceForgeMirror GHOSTSCRIPT "ghostscript/${SETUPFILE_GHOSTSCRIPT}"

!define DOWNLOAD_ASPELLDICTS "ftp://ftp.lyx.org/pub/lyx/contrib/aspell6-windows"
!define DOWNLOADALT_ASPELLDICTS "http://www.lyx.org/~bpeng/aspell6-windows"

#--------------------------------
# Locations of setup files for components (for bundled setup)

!define INSTALL_LATEX "${SETUPFILE_LATEX}"
!define INSTALL_IMAGEMAGICK "${SETUPFILE_IMAGEMAGICK}"
!define INSTALL_GHOSTSCRIPT "${SETUPFILE_GHOSTSCRIPT}"

#--------------------------------
# Names and version

!define APP_NAME "LyX"
!define /date APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}svn %Y%m%d"
!define APP_VERSION_NUMBER "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}.${APP_VERSION_BUILD}"
!define APP_SERIES_NAME "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}"
!define APP_SERIES_KEY "${APP_VERSION_MAJOR}${APP_VERSION_MINOR}"
!define APP_DIR "${APP_NAME}${APP_SERIES_KEY}"
!define APP_DIR_USERDATA "${APP_NAME}${APP_SERIES_KEY}"
!define APP_INFO "${APP_NAME} - The Document Processor"
!define APP_COPYRIGHT "LyX is Copyright © 1995 by Matthias Ettrich, 1995-2008 LyX Team"

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

!if ${SETUPTYPE} == MINIMAL

  !define SETUP_EXE ${MinimalExeFile}
 
!else if ${SETUPTYPE} == STANDARD

  !define SETUP_EXE ${ExeFile}
  !define BUNDLE_IMAGEMAGICK
  !define BUNDLE_GHOSTSCRIPT

!else if ${SETUPTYPE} == BUNDLE

  !define SETUP_EXE ${BundleExeFile}
  !define BUNDLE_IMAGEMAGICK
  !define BUNDLE_GHOSTSCRIPT
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
!define BIN_IMAGEMAGICK "convert.exe"
!define BIN_GHOSTSCRIPT "gswin32c.exe"

#--------------------------------
# Custom NSIS plug-ins

!addplugindir "${FILES_NSISPLUGINS}"
