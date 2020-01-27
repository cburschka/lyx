/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 2
!define APP_VERSION_MINOR 3
!define APP_VERSION_REVISION 2
!define APP_VERSION_EMERGENCY "" # use "1" for an emergency release of LyX otherwise ""
!define APP_EMERGENCY_DOT "" # use "." for an emergency release of LyX otherwise ""
!define APP_VERSION_BUILD 1 # Start with 1 for the installer releases of each version

!define APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}${APP_EMERGENCY_DOT}${APP_VERSION_EMERGENCY}" # Version to display

!define COPYRIGHT_YEAR 2019

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-232-Installer-1.exe" etc.

!ifndef ExeFile
  !define ExeFile "${APP_NAME}-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}-Installer-${APP_VERSION_BUILD}.exe"
!endif

#--------------------------------
# File locations
# !!! you need to adjust them to the folders in your Windows system !!!

!define FILES_BASE "C:\Users\rikih\LyX"
!define FILES_GIT "${FILES_BASE}\git\development\Win32\packaging\installer"
!define FILES_LYX "${FILES_BASE}\LyX23"
!define FILES_DEPS "${FILES_BASE}\Dependencies"
!define FILES_QT "${FILES_BASE}\LyX23"
!define ClassFileDir "${FILES_LYX}\Resources\tex"

#--------------------------------
# MiKTeX and JabRef

!define MiKTeXRepo "ftp://ftp.tu-chemnitz.de/pub/tex/systems/win32/miktex/tm/packages/"

!define ImageMagickVersion "7.0.7"
