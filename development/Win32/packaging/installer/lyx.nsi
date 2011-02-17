/*

NSIS Script - LyX 2.0 Installer for Win32
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Compatible with NSIS 2.46

*/

!include settings.nsh

#--------------------------------
# Header files  

!include include\declarations.nsh
!include include\variables.nsh
!include include\nsis.nsh

!include include\filelist.nsh

!include include\detection.nsh
!include include\gui.nsh
!include include\init.nsh

#--------------------------------
# The following commands use makensis to compile two applications.
# Although these applications are written in NSIS, they do not install anything.

# Compile the launcher
!system '"${NSISDIR}\makensis.exe" "${FILES_LAUNCHER}\launcher.nsi"'

# Compile Windows PDF view helper
!system '"${NSISDIR}\makensis.exe" "${FILES_PDFVIEW}\pdfview.nsi"'

#--------------------------------
# Setup

!include setup\install.nsh
!include setup\uninstall.nsh
!include setup\configure.nsh

#--------------------------------
# User interface (wizard pages)

!include gui\external.nsh
!include gui\reinstall.nsh

#--------------------------------
# Output file

Outfile "${SETUP_EXE}"
