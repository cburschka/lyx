#-------------------------------------------------------------
# - - - - - - - - - - - - Settings - - - - - - - - - - - - - -
#-------------------------------------------------------------
# TODO edit this section

!define VERSION_MAJOR 2
!define VERSION_MINOR 4
!define VERSION_REVISION 0
!define VERSION_EMERGENCY "1" # Empty string for regular releases, otherwise integer value
!define EMERGENCY_DOT "." # Empty string for regular releases, "." for emergency
!define VERSION_BUILD 1 # Only integer values
!define APP_ARCHITECTURE 64 # 32 or 64 bit

!define FILES_LYX "C:\lyx\masterbuild${APP_ARCHITECTURE}\LYX_INSTALLED" # Path to LYX_INSTALLED folder where bin\LyX.exe is compiled
!define FILES_QT "${FILES_LYX}" # Can choose same as FILES_LYX (if Qt files copied to LYX_INSTALLED folder after compiling) or Qt kit folder
!define FILES_DEPS "${FILES_LYX}" # Can choose same as FILES_LYX (if dependencies copied to LYX_INSTALLED folder after compiling) or the dependencies folder itself (GNUWIN32 dir)

!define Qt_Version 6 # 5 or 6 (4 is not supported)

!define COPYRIGHT_YEAR 2020

#-------------------------------------------------------------
# - - - - - - - - - - - - Sourcecode - - - - - - - - - - - - -
#-------------------------------------------------------------
# Do not edit

!define APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}${EMERGENCY_DOT}${VERSION_EMERGENCY}"
!define APP_VERSION_DOTLESS "${VERSION_MAJOR}${VERSION_MINOR}${VERSION_REVISION}${VERSION_EMERGENCY}"

OutFile "LyX-${APP_VERSION_DOTLESS}-Installer-${VERSION_BUILD}-x${APP_ARCHITECTURE}.exe"

!include "src\main.nsh"