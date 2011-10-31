/*

NSIS Script - LyX 2.0 Installer for Win32
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Compatible with NSIS 2.46

*/

!include settings.nsh

!include include\declarations.nsh
!include include\variables.nsh
!include include\nsis.nsh
!include include\filelist.nsh
!include include\detection.nsh
!include include\gui.nsh
!include include\init.nsh
!include setup\install.nsh
!include setup\uninstall.nsh
!include setup\configure.nsh
!include gui\external.nsh

#--------------------------------
# Output file

Outfile "${SETUP_EXE}"
