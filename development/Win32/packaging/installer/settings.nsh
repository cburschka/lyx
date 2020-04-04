/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 2
!define APP_VERSION_MINOR 4
!define APP_VERSION_REVISION 0
!define APP_VERSION_EMERGENCY "dev" # use "1" for an emergency release of LyX otherwise ""
!define APP_EMERGENCY_DOT "" # use "." for an emergency release of LyX otherwise ""
!define APP_VERSION_BUILD 1 # Start with 1 for the installer releases of each version

# 32 or 64 bit, use build folder names and dependencies folder names containing "32" or "64" to use following constant in File locations declarations below
!define APP_VERSION_ACHITECHTURE 32

!define APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_REVISION}${APP_EMERGENCY_DOT}${APP_VERSION_EMERGENCY}" # Version to display

!define COPYRIGHT_YEAR 2020

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-232-Installer-1.exe" etc.

!ifndef ExeFile
  !define ExeFile "${APP_NAME}-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}${APP_VERSION_REVISION}${APP_VERSION_EMERGENCY}-Installer-${APP_VERSION_BUILD}-\
  x${APP_VERSION_ACHITECHTURE}.exe"
!endif

#--------------------------------
# File locations
# !!! you need to adjust them to the folders in your Windows system !!!

!define FILES_BASE "C:\lyx"
!define FILES_GIT "${FILES_BASE}\master\development\Win32\packaging"
!define FILES_LYX "${FILES_BASE}\masterbuild${APP_VERSION_ACHITECHTURE}\LYX_INSTALLED"
!define FILES_DEPS "${FILES_BASE}\lyx-windows-deps-msvc2019_${APP_VERSION_ACHITECHTURE}"
!define FILES_QT "${FILES_BASE}\masterbuild${APP_VERSION_ACHITECHTURE}\LYX_INSTALLED"
!define ClassFileDir "${FILES_LYX}\Resources\tex"

#--------------------------------
# MiKTeX and JabRef

!define MiKTeXRepo "ftp://ftp.tu-chemnitz.de/pub/tex/systems/win32/miktex/tm/packages/"

!define ImageMagickVersion "7.0.10"
