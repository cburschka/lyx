/*

Declarations

*/

!include "settings.nsh"

;--------------------------------
;Defines based on settings

!ifndef SETUPTYPE_BUNDLE
  !define SETUPTYPE_NAME DOWNLOAD
!else
  !define SETUPTYPE_NAME INSTALL
!endif

;--------------------------------
;Standard header files

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "WordFunc.nsh"
!include "Sections.nsh"

;--------------------------------
;Windows constants

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0x0000

;--------------------------------
;Reserve Files
;These files should come first in the compressed data (for faster GUI)

ReserveFile "${NSISDIR}\Plugins\UserInfo.dll"
ReserveFile "dialogs\user.ini"
ReserveFile "dialogs\external.ini"
ReserveFile "dialogs\viewer.ini"
ReserveFile "dialogs\langselect.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Variables

Var AdminOrPowerUser
Var AllUsersInstall

Var PathLaTeX
Var PathImageMagick
Var PathGhostscript
Var PathViewer

Var SetupLaTeX
Var SetupImageMagick
Var SetupGhostscript
Var SetupViewer

Var SizeLaTeX
Var SizeImageMagick
Var SizeGhostscript
Var SizeViewer

Var LangName
Var LangCode

Var AspellHive

;--------------------------------
;Generic defines

!define FALSE 0
!define TRUE 1
  
;--------------------------------
;Include standard functions

!insertmacro GetParent
!insertmacro VersionCompare

;--------------------------------
;Macros  

!macro ReDef name value

  ;Redefine a pre-processor definition

  !ifdef `${name}`
    !undef `${name}`
  !endif

  !define `${name}` `${value}`

!macroend
  
!macro CallFunc function input var_output
  
  ;Calls a function that modifies a single value on the stack

  Push ${input}
    Call ${function}
  Pop ${var_output}

!macroend

;--------------------------------
;LyX installer header files  

!include "include\windows.nsh"
!include "include\lang.nsh"
!include "include\gui.nsh"
!include "include\detection.nsh"
!include "include\filelists.nsh"
