﻿/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 2
!define APP_VERSION_MINOR 3
!define APP_VERSION_REVISION 0
!define APP_VERSION_EMERGENCY "RC-1" # use "1" for an emergency release of LyX otherwise ""
!define APP_EMERGENCY_DOT "" # use "." for an emergency release of LyX otherwise ""
!define APP_VERSION_BUILD 6 # Start with 1 for the installer releases of each version

!define APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}${APP_EMERGENCY_DOT}${APP_VERSION_EMERGENCY}" # Version to display

!define COPYRIGHT_YEAR 2017

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-211-Installer-1.exe" etc.

!ifndef ExeFile
  !define ExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}-Installer-${APP_VERSION_BUILD}.exe"
!endif
!ifndef BundleExeFile
  !define BundleExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}-Bundle-${APP_VERSION_BUILD}.exe"
!endif

#--------------------------------
# File locations
# !!! you need to adjust them to the folders in your Windows system !!!

!define FILES_LYX "D:\LyXPackage2.3\LyX"
!define FILES_DEPS "D:\LyXGit\Master\lyx-windows-deps-msvc2015"
!define FILES_QT "C:\Qt\Qt5.9.3\5.9.3\msvc2015"
!define ClassFileDir "${FILES_LYX}\Resources\tex"
!define DVIPostFileDir "${FILES_DEPS}\tex"

#--------------------------------
# MiKTeX and JabRef

!define MiKTeXRepo "ftp://ftp.fernuni-hagen.de/pub/mirrors/www.ctan.org/systems/win32/miktex/tm/packages/"

!define MiKTeXDeliveredVersion "2.9"
!define ImageMagickVersion "7.0.7"
 
# definitions for the Complete installer
!if ${SETUPTYPE} == BUNDLE
 !define MiKTeXInstall "$INSTDIR\external\basic-miktex-2.9.6520.exe"
!endif

