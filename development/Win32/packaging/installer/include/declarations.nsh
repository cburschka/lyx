/*

declaration.nsh

Configuration of LyX installer

*/

#--------------------------------
# File locations

!define FILES_LICENSE "${FILES_GIT}\installer\license.rtf"
!define FILES_ICONS "${FILES_GIT}\icons"
!define FILES_PDFVIEW "${FILES_DEPS}\bin"
!define FILES_MSVC "${FILES_DEPS}\bin"
!define FILES_PERL "${FILES_DEPS}\Perl"
!define FILES_PYTHON "${FILES_DEPS}\Python"
!define FILES_UNOCONV "${FILES_DEPS}\bin"
!define FILES_IMAGEMAGICK "${FILES_DEPS}\imagemagick"
!define FILES_GHOSTSCRIPT "${FILES_DEPS}\ghostscript"
!define FILES_NETPBM "${FILES_DEPS}\bin"
!define FILES_RSVG "${FILES_DEPS}\bin"
!define FILES_DTL "${FILES_DEPS}\bin"
!define FILES_PDFTOOLS "${FILES_DEPS}\bin"
!define FILES_METAFILE2EPS "${FILES_DEPS}\bin"

#--------------------------------
# Locations of components to download

# CTAN and SourceForge select a mirror automatically

!define DOWNLOAD_LATEX "http://mirrors.ctan.org/systems/win32/miktex/setup/${SETUPFILE_LATEX}"

#--------------------------------
# Names and version

!define APP_NAME "LyX"
!define APP_NAME_SMALL "lyx"
!define APP_VERSION_NUMBER "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}.${APP_VERSION_BUILD}"
# for the proposed install folder we use the scheme "LyX 2.2" while we need for the registry the scheme "LyX 2.1.4"
# to check if it is exactly this version (to support side by side installations)
!define APP_SERIES_NAME "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}"
!define APP_SERIES_KEY "${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}"
!define APP_SERIES_KEY2 "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}${APP_EMERGENCY_DOT}${APP_VERSION_EMERGENCY}"
!define APP_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${APP_NAME}.exe"
!define APP_DIR "${APP_NAME} ${APP_SERIES_NAME}"
!define APP_DIR_USERDATA "${APP_NAME}${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}"
!define APP_INFO "${APP_NAME} - The Document Processor"
!define APP_WEBPAGE "https://www.lyx.org/"
!define APP_WEBPAGE_INFO "${APP_NAME} Website"
!define APP_WIKI "https://wiki.lyx.org"
!define APP_WIKI_INFO "${APP_NAME} Wiki"
!define APP_COPYRIGHT "${APP_NAME} is Copyright © 1995 by Matthias Ettrich, 1995-${COPYRIGHT_YEAR} by the ${APP_NAME} Team"

!if ${SETUPTYPE} == STANDARD
  !define APP_SETUPTYPE "Standard"
!endif

!define BIN_LYX "${APP_NAME}.exe"
!define APP_RUN "bin\${BIN_LYX}"

!define APP_REGFOLDER "Software\${APP_NAME}"
!define APP_REGKEY "${APP_REGFOLDER}\${APP_SERIES_KEY}" # like "LyX\220"
!define APP_REGKEY_SETUP "${APP_REGKEY}\Setup"
!define APP_REGKEY_SETTINGS "${APP_REGKEY}\Settings"

!define APP_REGNAME_DOC "${APP_NAME}.Document"

!define APP_EXT ".${APP_NAME_SMALL}"
!define APP_MIME_TYPE "application/${APP_NAME_SMALL}"

!define APP_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}"

#--------------------------------
# Setup settings

# Output file name can be configured using command line paramaters like
# /DExeFile=/path/to/installer

!if ${SETUPTYPE} == STANDARD
  !define SETUP_EXE ${ExeFile}
!endif

!define SETUP_ICON "${FILES_ICONS}\${APP_NAME_SMALL}.ico"
!define SETUP_HEADERIMAGE "graphics\header.bmp"
!define SETUP_WIZARDIMAGE "graphics\wizard.bmp"
!define SETUP_UNINSTALLER "Uninstall-${APP_NAME}.exe"
!define SETUP_UNINSTALLER_KEY "${APP_NAME}${APP_SERIES_KEY}"

#--------------------------------
# Names of binaries to identify components

!define BIN_LATEX "latex.exe"
!define BIN_BIBTEXEDITOR "JabRef.exe"
