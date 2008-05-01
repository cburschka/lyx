/*

Settings for LyX installer

These typically need to be modified for each LyX release

*/

#--------------------------------
# Version number

!define APP_VERSION_MAJOR 1
!define APP_VERSION_MINOR 5
!define APP_VERSION_REVISION 5
!define APP_VERSION_BUILD 0 # Start with 1 for the installer releases of each version

#--------------------------------
# Installer file name

# Typical names for the release are "LyX-1.5.4-1-Installer.exe" etc.

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
# File names of component installers

!define SETUPFILE_LATEX "basic-miktex.2.7.2960.exe"
!define SETUPFILE_IMAGEMAGEMAGICK "ImageMagick-6.4.1-0-Q16-windows-dll.exe"
!define SETUPFILE_GHOSTSCRIPT "gs861w32.exe"

#--------------------------------
# Download size of components (in KB)

!define SIZE_DOWNLOAD_LATEX 78493
!define SIZE_DOWNLOAD_IMAGEMAGICK 8565
!define SIZE_DOWNLOAD_GHOSTSCRIPT 12469

#--------------------------------
# Approximations of space required for components (in KB)

!define SIZE_LATEX 225000
!define SIZE_IMAGEMAGICK 34500
!define SIZE_GHOSTSCRIPT 31500
