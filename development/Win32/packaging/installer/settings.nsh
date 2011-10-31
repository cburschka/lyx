/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 2
!define APP_VERSION_MINOR 0
!define APP_VERSION_REVISION 0
!define APP_VERSION_BUILD 0 # Start with 1 for the installer releases of each version

!define /date APP_VERSION "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}svn %Y%m%d" # Version to display

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-2.0.0-1-Installer.exe" etc.

!ifndef ExeFile
  !define /date ExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}svn-%Y%m%d-Installer.exe"
!endif
!ifndef BundleExeFile
  !define /date BundleExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}svn-%Y%m%d-Installer-Bundle.exe"
!endif
!ifndef MinimalExeFile
  !define /date MinimalExeFile "LyX-${APP_VERSION_MAJOR}${APP_VERSION_MINOR}svn-%Y%m%d-Installer-Minimal.exe"
!endif

#--------------------------------
# File locations

!define FILES_LYX "C:\LyX\lyx-20-install"
!define FILES_DEPS "C:\LyX\lyx-20-build\msvc2010-deps\deps20"
!define FILES_BUNDLE "C:\LyX\depsbundle"
!define FILES_QT "C:\Qt\qt-everywhere-opensource-src-4.7.4"

#--------------------------------
# MiKTeX
# Sizes in KB

!define SETUPFILE_LATEX "basic-miktex-2.9.4250.exe"
!define SIZE_DOWNLOAD_LATEX 167828
!define SIZE_LATEX 380000
