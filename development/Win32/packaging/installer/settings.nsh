/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 2
!define APP_VERSION_MINOR 0
!define APP_VERSION_REVISION 5
!define APP_VERSION_EMERGENCY "1" # use "1" for an emergency release of LyX otherwise ""
!define APP_EMERGENCY_DOT "." # use "." for an emergency release of LyX otherwise ""
!define APP_VERSION_BUILD 1 # Start with 1 for the installer releases of each version

!define APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}${APP_EMERGENCY_DOT}${APP_VERSION_EMERGENCY}" # Version to display

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-201-Installer-1.exe" etc.

!ifndef ExeFile
  !define ExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}-Installer-${APP_VERSION_BUILD}.exe"
!endif
!ifndef BundleExeFile
  !define BundleExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}-Bundle-${APP_VERSION_BUILD}.exe"
!endif
!ifndef MinimalExeFile
  !define /date MinimalExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}svn-%Y%m%d-Installer-Minimal.exe"
!endif

#--------------------------------
# File locations
# !!! you need to adjust them to the folders in your Windows system !!!

!define FILES_LYX "C:\LyX\lyx-20-install"
!define FILES_DEPS "C:\LyX\lyx-20-build\msvc2010-deps\deps20"
!define FILES_BUNDLE "C:\LyX\depsbundle"
!define FILES_QT "C:\Qt\qt-everywhere-opensource-src-4.8.3"
!define ClassFileDir "${FILES_LYX}\Resources\tex"
!define DVIPostFileDir "${FILES_DEPS}\tex"

#--------------------------------
# MiKTeX
# Sizes in KB

# it seems that some companies block ftp access by default, therefore http access is preferred here
!define MiKTeXRepo "http://ftp.fernuni-hagen.de/ftp-dir/pub/mirrors/www.ctan.org/systems/win32/miktex/tm/packages/"
#!define MiKTeXRepo "ftp://ftp.tu-chemnitz.de/pub/tex/systems/win32/miktex/tm/packages/"

!define JabRefVersion "2.8.1"
!define MiKTeXDeliveredVersion "2.9"
!define ImageMagickVersion "6.8.0"
 
# definitions for the Complete installer
!if ${SETUPTYPE} == BUNDLE
 !define JabRefInstall "external\JabRef-2.8.1-setup.exe"
 !define SIZE_JABREF 12400
 !define MiKTeXInstall "$INSTDIR\external\basic-miktex-2.9.4521.exe"
 #!define SIZE_DOWNLOAD_LATEX 157100
 #!define SIZE_LATEX 600000
!endif

