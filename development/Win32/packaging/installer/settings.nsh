/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 2
!define APP_VERSION_MINOR 1
!define APP_VERSION_REVISION 4
!define APP_VERSION_EMERGENCY "" # use "1" for an emergency release of LyX otherwise ""
!define APP_EMERGENCY_DOT "" # use "." for an emergency release of LyX otherwise ""
!define APP_VERSION_BUILD 6 # Start with 1 for the installer releases of each version

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

!define FILES_LYX "D:\LyXPackage2.1\LyX"
!define FILES_DEPS "D:\LyXGit\Master\lyx-windows-deps-msvc2010"
!define FILES_QT "C:\Qt\4.8.7"
!define ClassFileDir "${FILES_LYX}\Resources\tex"
!define DVIPostFileDir "${FILES_DEPS}\tex"

#--------------------------------
# MiKTeX and JabRef
# Sizes in KB

"ftp://ftp.fernuni-hagen.de/pub/mirrors/www.ctan.org/systems/win32/miktex/tm/packages/"

!define JabRefVersion "2.11"
!define MiKTeXDeliveredVersion "2.9"
!define ImageMagickVersion "6.9.4"
 
# definitions for the Complete installer
!if ${SETUPTYPE} == BUNDLE
 !define JabRefInstall "external\JabRef-2.11-setup.exe"
 !define SIZE_JABREF 14100
 !define MiKTeXInstall "$INSTDIR\external\basic-miktex-2.9.5872.exe"
!endif

