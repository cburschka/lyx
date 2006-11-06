/*

NSIS Script - LyX 1.4 Installer for Win32
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Requires NSIS 2.21 or later

Licence details for all installer scripts can be found
in the file COPYING or at http://www.lyx.org/about/license.php

*/

SetCompressor /SOLID lzma

!include "include\declarations.nsh"

;--------------------------------
;Compile the launcher

!system '"${NSISDIR}\makensis.exe" "${FILES_LAUNCHER}\launcher.nsi"'

;--------------------------------
;Compile Windows PDF view helper

!system '"${NSISDIR}\makensis.exe" "${FILES_PDFVIEW}\pdfview.nsi"'

;--------------------------------
;LyX Installer

OutFile "${SETUP_EXE}"

;--------------------------------
;Components

!include "components\core.nsh"
!include "components\user.nsh"
!include "components\reinstall.nsh"
!include "components\external.nsh"
!include "components\viewer.nsh"
!include "components\dicts.nsh"
!include "components\langselect.nsh"
!include "components\configure.nsh"
!include "components\uninstall.nsh"
