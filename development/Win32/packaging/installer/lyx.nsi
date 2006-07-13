/*

NSIS Script - LyX 1.4 Installer for Win32
Authors: Joost Verburg, Angus Leeming, Uwe Stöhr
Requires NSIS 2.16 or later

Licence details for all installer scripts can be found
in the file COPYING or at http://www.lyx.org/about/license.php

*/
 
!include "include\declarations.nsh"

OutFile "${SETUP_EXE}"

;--------------------------------
;Functions

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  Call CheckWindows
  Call SearchAll
FunctionEnd

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
  Call un.SetShellContext
FunctionEnd

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
