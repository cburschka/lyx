/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR @LYX_MAJOR_VERSION@
!define APP_VERSION_MINOR @LYX_MINOR_VERSION@
!define APP_VERSION_PATCH @LYX_PATCH_VERSION@
!define APP_VERSION_REVISION @LYX_REVISION_VERSION@
#!define APP_VERSION_BUILD 0 # isn't revision number enough?

!define /date APP_VERSION "@LYX_MAJOR_VERSION@.@LYX_MINOR_VERSION@.@LYX_PATCH_VERSION@-@LYX_REVISION_VERSION@" # Version to display

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-2.0.0-1-Installer.exe" etc.

!ifndef ExeFile
  !define /date ExeFile "LyX-${APP_VERSION}-Installer.exe"
!endif
!ifndef BundleExeFile
  !define /date BundleExeFile "LyX-${APP_VERSION}-Installer-Bundle.exe"
!endif
!ifndef MinimalExeFile
  !define /date MinimalExeFile "LyX-${APP_VERSION}-Installer-Minimal.exe"
!endif

#--------------------------------
# File locations

!define FILES_LYX "@CMAKE_INSTALL_PREFIX_ABSOLUTE@"
!define FILES_DEPS "@LYX_DEPENDENCIES_DIR@/deps20"
!define FILES_BUNDLE "@CMAKE_BINARY_DIR@/depsbundle" #TODO Where is it? Where does it comes from?
!define FILES_QT "@QT_INCLUDE_DIR@/.."
!define FILES_INSTALLER "@CMAKE_SOURCE_DIR@/development/Win32/packaging/installer"

#--------------------------------
# MiKTeX
# Sizes in KB

!define SETUPFILE_LATEX "basic-miktex-2.9.4146.exe"
!define SIZE_DOWNLOAD_LATEX 160106
!define SIZE_LATEX 380000
