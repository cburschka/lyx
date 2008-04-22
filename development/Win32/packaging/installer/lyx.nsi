/*

NSIS Script - LyX 1.5/1.6 Installer for Win32
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Requires NSIS 2.36 or later

Licence details for all installer scripts can be found in the file COPYING

*/

!include settings.nsh

#--------------------------------
# The following commands use makensis to compile two applications.
# Although these applications are written in NSIS, they do not install anything.

# Compile the launcher
!system '"${NSISDIR}\makensis.exe" "${FILES_LAUNCHER}\launcher.nsi"'

# Compile Windows PDF view helper
!system '"${NSISDIR}\makensis.exe" "${FILES_PDFVIEW}\pdfview.nsi"'

#--------------------------------
# Output file can be specified using command line option
# /DExeFile=/path/to/installer or /DBundleExeFile=/path/to/installer if 
# SETUPTYPE_BUNDLE is defined.

!ifdef SETUPTYPE_BUNDLE
  # Bundle installer
  !ifdef BundleExeFile
    OutFile "${BundleExeFile}"
  !else
    Outfile "${SETUP_EXE}"
 !endif 
!else
  # Regular installer
  !ifdef ExeFile
    OutFile "${ExeFile}"
  !else
    Outfile "${SETUP_EXE}"
  !endif
!endif

#--------------------------------
# Header files  

!include include\declarations.nsh
!include include\detection.nsh
!include include\dictlist.nsh
!include include\filelist.nsh
!include include\gui.nsh
!include include\init.nsh
!include include\langlist.nsh

#--------------------------------
# Setup

!include setup\install.nsh
!include setup\uninstall.nsh
!include setup\configure.nsh

#--------------------------------
# User interface (wizard pages)

!include gui\external.nsh
!include gui\langselect.nsh
!include gui\reinstall.nsh
