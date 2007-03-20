/*

NSIS Script - LyX 1.4 Installer for Win32
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Requires NSIS 2.21 or later

Licence details for all installer scripts can be found
in the file COPYING or at http://www.lyx.org/about/license.php

*/

SetCompressor /SOLID lzma

!include "include\declarations.nsh"

#--------------------------------
# The following commands use makensis to compile two applications.
# Although these applications are written in NSIS, they do not install anything.

#--------------------------------
#Compile the launcher

!system '"${NSISDIR}\makensis.exe" "${FILES_LAUNCHER}\launcher.nsi"'

#--------------------------------
#Compile Windows PDF view helper

!system '"${NSISDIR}\makensis.exe" "${FILES_PDFVIEW}\pdfview.nsi"'

#--------------------------------
#LyX Installer, output file can be specified using command line option
# /DExeFile=/path/to/installer or /DBundleExeFile=/path/to/installer if 
# SETUPTYPE_BUNDLE is defined.

# bundle installer
!ifdef SETUPTYPE_BUNDLE
!ifdef BundleExeFile
OutFile "${BundleExeFile}"
!else
Outfile "${SETUP_EXE}"
!endif
# regular installer 
!else
!ifdef ExeFile
OutFile "${ExeFile}"
!else
Outfile "${SETUP_EXE}"
!endif
!endif

#--------------------------------
#Components

!include "components\core.nsh"
!include "components\user.nsh"
!include "components\reinstall.nsh"
!include "components\external.nsh"
!include "components\viewer.nsh"
!include "components\dicts.nsh"
!include "components\langselect.nsh"
!include "components\configure.nsh"
!include "components\uninstall.nsh"
