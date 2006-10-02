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
  ${unless} ${silent}
    Banner::show /NOUNLOAD "Checking system"
  ${endif}
  
  Call CheckWindows
  Call CheckPrivileges
  Call InitInstaller
  Call SearchAll

  ${unless} ${silent}
    Banner::destroy
  ${endif}
FunctionEnd

Function un.onInit
  Call un.CheckPrivileges
  Call un.InitUnInstaller
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
