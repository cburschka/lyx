; Lyx for Windows, NSIS v2 series installer script

; File LyXInstaller-update.nsi
; This file is part of LyX, the document processor.
; http://www.lyx.org/
; Licence details can be found in the file COPYING or copy at
; http://www.lyx.org/about/license.php3

; Author Uwe Stöhr
; Full author contact details are available in file CREDITS or copy at
; http://www.lyx.org/about/credits.php

; This script requires NSIS 2.25 and newer
; http://nsis.sourceforge.net/
;--------------------------------

; Do a Cyclic Redundancy Check to make sure the installer
; was not corrupted by the download.
CRCCheck force

; Make the installer as small as possible.
SetCompressor lzma

;--------------------------------
; You should need to change only these macros...

!define INSTALLER_VERSION "Update"
!define INSTALLER2_VERSION "Small"
!define INSTALLER3_VERSION "Complete"

; load the settings
!include "Settings.nsh"

!define PRODUCT_UNINST_KEY_OLD "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX150svn"
!define PRODUCT_VERSION_OLD "LyX 1.5beta2-21-04-2007"
!define PRODUCT_UNINSTALL_EXE_OLD "$INSTDIR\LyXWinUninstall.exe"

;--------------------------------
; variables only used in this installer version

Var INSTDIR_NEW

;--------------------------------
; load some NSIS libraries
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"
!include "TextFunc.nsh"
!insertmacro LineFind
!include "WordFunc.nsh"
!insertmacro WordReplace

; Set of various macros and functions
!include "LyXUtils.nsh"

; Use the Abiword macros to help set up associations with the file extension in the Registry.
; Grabbed from
; http://abiword.pchasm.org/source/cvs/abiword-cvs/abi/src/pkg/win/setup/NSISv2/abi_util_fileassoc.nsh
!include "abi_util_fileassoc.nsh"

; list with modified files
!include "Updated.nsh"

; list with deleted files
!include "Deleted.nsh"

;--------------------------------

; Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${PRODUCT_DIR}\icons\lyx_logo_hi.bmp"
!define MUI_ICON "${PRODUCT_DIR}\icons\lyx.ico"
!define MUI_UNICON "${PRODUCT_DIR}\icons\lyx.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${VERSION_BITMAP}"

; Welcome page
!define MUI_WELCOMEPAGE_TEXT "$(WelcomePageText)"
!insertmacro MUI_PAGE_WELCOME

; Show the license.
!insertmacro MUI_PAGE_LICENSE "${PRODUCT_LICENSE_FILE}"

; Define which components to install.
!insertmacro MUI_PAGE_COMPONENTS

; Specify where to install program shortcuts.
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "LyX ${PRODUCT_VERSION}"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $StartmenuFolder

; Watch the components being installed.
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_RUN 
!define MUI_FINISHPAGE_TEXT "$(FinishPageMessage)"
!define MUI_FINISHPAGE_RUN_TEXT "$(FinishPageRun)"
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchProduct"
!insertmacro MUI_PAGE_FINISH

; The uninstaller.
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English" ; first language is the default language
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Turkish"

!include "lyx_languages\english.nsh"
!include "lyx_languages\danish.nsh"
!include "lyx_languages\dutch.nsh"
!include "lyx_languages\french.nsh"
!include "lyx_languages\german.nsh"
!include "lyx_languages\galician.nsh"
!include "lyx_languages\hungarian.nsh"
!include "lyx_languages\italian.nsh"
!include "lyx_languages\norwegian.nsh"
!include "lyx_languages\polish.nsh"
!include "lyx_languages\portuguese.nsh"
!include "lyx_languages\slovak.nsh"
!include "lyx_languages\spanish.nsh"
!include "lyx_languages\swedish.nsh"
!include "lyx_languages\turkish.nsh"

LicenseData "$(LyXLicenseData)"

;--------------------------------
; Reserve Files

; These files are inserted before other files in the data block
!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
; Installer sections

Section "!${PRODUCT_NAME}" SecCore
  SectionIn RO
SectionEnd
Section "$(SecDesktopTitle)" SecDesktop
  StrCpy $CreateDesktopIcon "true"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(SecCoreDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; the installation section
!include "InstallActions-update.nsh"

;--------------------------------
; This hook function is called internally by NSIS on installer startup
Function .onInit

  ; Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}
  
  ; set the installer language to the Windows locale language
  System::Call "kernel32::GetUserDefaultLangID()i.a"
  
  ; Check that the installer is not currently running
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${INSTALLER_EXE}.Instance") i .r1 ?e'
  Pop $R0
  ${if} $R0 != "0"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)"
   Abort
  ${endif}
  FindProcDLL::FindProc "${INSTALLER2_EXE}"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)"
   Abort
  ${endif}
  FindProcDLL::FindProc "${INSTALLER3_EXE}"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)"
   Abort
  ${endif}
  
  ; check where LyX is installed
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY_OLD}" "DisplayIcon"
  ${if} $0 != ""
    SetShellVarContext all
    StrCpy $ProductRootKey "HKLM"
  ${endif}
  ${if} $0 == ""
   ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY_OLD}" "DisplayIcon"
   ${if} $0 != ""
    SetShellVarContext current
    StrCpy $ProductRootKey "HKCU"
   ${endif}
   ${if} $0 == ""
    MessageBox MB_OK|MB_ICONSTOP "$(UpdateNotAllowed)"
    Abort
   ${endif}
  ${endif} ; end if $0 == 0
  ${if} $0 != "" ; if it is found
   StrCpy $INSTDIR $0
   StrCpy $INSTDIR $INSTDIR -12 ; delete the string "\bin\lyx.exe" or "\bin\lyx.bat"
  ${endif}
  
  ; abort if the user doesn't have administrator privileges but LyX was installed as admin
  StrCpy $Answer ""
  StrCpy $UserName ""
  !insertmacro IsUserAdmin $Answer $UserName ; macro from LyXUtils.nsh
  ${if} $Answer != "yes"
  ${andif} $ProductRootKey == "HKLM"
    MessageBox MB_OK|MB_ICONSTOP "$(NotAdmin)"
    Abort
  ${endif}
  
  ; This can be reset to "true" in section SecDesktop.
  StrCpy $CreateDesktopIcon "false"
  
  ClearErrors
FunctionEnd

;--------------------------------

Function LaunchProduct
  Exec ${PRODUCT_BAT}
FunctionEnd

;--------------------------------
; The Uninstaller

Function un.onInit
	
  ; Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}

  ; set registry root key
  StrCpy $Answer ""
  !insertmacro IsUserAdmin $Answer $UserName ; macro from LyXUtils.nsh
  ${if} $Answer == "yes"
    SetShellVarContext all
  ${else}
   SetShellVarContext current
  ${endif}

  ; Ascertain whether the user has sufficient privileges to uninstall.
  ; abort when LyX was installed with admin permissions but the user doesn't have administrator privileges
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "RootKey"
  ${if} $0 != ""
  ${andif} $Answer != "yes"
   MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)"
   Abort
  ${endif}
  ; abort when LyX couldn't be found in the registry
  ${if} $0 == "" ; check in HKCU
   ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY}" "RootKey"
   ${if} $0 == ""
     MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)"
   ${endif}
  ${endif}
  
FunctionEnd

Function un.onUninstSuccess

 HideWindow ; hides the uninstaller Window when it is ready

FunctionEnd

Section "un.LyX" un.SecUnProgramFiles

  SectionIn RO

  ; delete LaTeX class files that were installed together with LyX
  FileOpen $R5 "$INSTDIR\Resources\uninstallPaths.dat" r
  FileRead $R5 $LatexPath
  FileClose $R5
  StrCpy $String $LatexPath
  StrCpy $Search "miktex\bin"
  StrLen $3 $String
  Call un.StrPoint ; search the LaTeXPath for the phrase "miktex\bin" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" ; if something was found
   IntOp $Pointer $Pointer - 1 ; jump before the first "\" of "\miktex\bin"
   StrCpy $String $String "$Pointer" ; $String is now the part before "\miktex\bin"
   Delete "$String\tex\latex\cv.cls"
   RMDir /r "$String\tex\latex\lyx"
   RMDir /r "$String\tex\latex\revtex"
   RMDir /r "$String\tex\latex\hollywood"
   RMDir /r "$String\tex\latex\broadway"
   ExecWait "$LatexPath\initexmf --update-fndb"
  ${endif}

  ; delete start menu folder
  ReadRegStr $0 SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu"
  RMDir /r "$0"
  ; delete desktop icon
  Delete "$DESKTOP\LyX ${PRODUCT_VERSION}.lnk"
  ; delete registry entries
  DeleteRegKey HKCU "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  
  ; run the installer of the old LyX version to clean up the third party products
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "RootKey" "Start" ; dummy entry to let the uninstaller start
  Exec "${PRODUCT_UNINSTALL_EXE_OLD}"
  
SectionEnd


; eof
