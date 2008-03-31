/*

Declarations

*/

!include "settings.user.nsh"
!include "settings.nsh"

#--------------------------------
#Defines based on settings

!ifndef SETUPTYPE_BUNDLE
  !define SETUPTYPE_NAME DOWNLOAD
!else
  !define SETUPTYPE_NAME INSTALL
!endif

#--------------------------------
#Standard header files

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "StrFunc.nsh"
!include "FileFunc.nsh"
!include "WordFunc.nsh"
!include "Sections.nsh"
!include "WinVer.nsh"

#--------------------------------
#Windows constants

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0x0000

#--------------------------------
#Windows Vista settings

RequestExecutionLevel highest

#--------------------------------
#Reserve Files
#These files should come first in the compressed data (for faster GUI)

ReserveFile "${NSISDIR}\Plugins\UserInfo.dll"
ReserveFile "dialogs\user.ini"
ReserveFile "dialogs\external.ini"
ReserveFile "dialogs\viewer.ini"
ReserveFile "dialogs\langselect.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
ReserveFile "${FILES_DVIPOST_PKG}\dvipost.sty"

#--------------------------------
#Variables

Var AdminOrPowerUser
Var CurrentUserInstall

Var PathLaTeX
Var PathLaTeXLocal
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
Var LangISOCode

Var AspellHive
  
#--------------------------------
#Include standard functions

!insertmacro FUNCTION_STRING_StrStr

!insertmacro GetParent
!insertmacro VersionCompare
!insertmacro WordFind2X

#--------------------------------
#Macros  

!macro ReDef NAME VALUE

  #Redefine a pre-processor definition

  !ifdef `${NAME}`
    !undef `${NAME}`
  !endif

  !define `${NAME}` `${VALUE}`

!macroend
  
!macro CallFunc FUNCTION INPUT VAROUT
  
  #Calls a function that modifies a single value on the stack

  Push ${INPUT}
    Call ${FUNCTION}
  Pop ${VAROUT}

!macroend

#--------------------------------
#LyX installer header files  

!include "include\download.nsh"
!include "include\init.nsh"
!include "include\windows.nsh"
!include "include\lang.nsh"
!include "include\gui.nsh"
!include "include\detection.nsh"
!include "include\filelists.nsh"
