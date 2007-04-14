; Lyx for Windows, NSIS v2 series installer script

; File LyX-Installer-Update.nsi
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
; SetCompressor lzma

;--------------------------------
; You should need to change only these macros...

!define PRODUCT_DIR "D:\LyXPackage1.5"
!define PRODUCT_NAME "LyX"
!define PRODUCT_VERSION "1.5svn-xx-04-2007"
!define PRODUCT_VERSION_SHORT "150svn"
!define PRODUCT_SUBFOLDER "lyx15"
!define PRODUCT_LICENSE_FILE "${PRODUCT_DIR}\License.txt"
!define PRODUCT_SOURCEDIR "${PRODUCT_DIR}\LyX"
!define PRODUCT_EXE "$INSTDIR\bin\lyx.exe"
!define PRODUCT_BAT "$INSTDIR\bin\lyx.bat"
!define PRODUCT_EXT ".lyx"
!define PRODUCT_MIME_TYPE "application/lyx"
!define PRODUCT_UNINSTALL_EXE "$INSTDIR\LyXWinUninstall.exe"
!define PRODUCT_HELP_LINK "http://www.lyx.org/internet/mailing.php"
!define PRODUCT_ABOUT_URL "http://www.lyx.org/about/"
!define PRODUCT_INFO_URL "http://www.lyx.org/"

BrandingText "LyXWinInstaller v3.12 - Update"
!define INSTALLER_VERSION "Small"
!define INSTALLER_EXE "LyXWin150svnUpdate-3-12.exe"
!define INSTALLER2_EXE "LyXWin150svnSmall-3-12.exe" ; to check later if this installer version is running at the same time
!define INSTALLER3_EXE "LyXWin150svnComplete-3-12.exe" ; to check later if this installer version is running at the same time
!define VERSION_BITMAP "${PRODUCT_DIR}\icons\lyx_logo_vert${PRODUCT_VERSION_SHORT}.bmp"

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\lyx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX${PRODUCT_VERSION_SHORT}"
!define PRODUCT_UNINST_KEY_OLD "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX150svn"
!define PRODUCT_VERSION_OLD "LyX 1.5svn-09-04-2007"
!define PRODUCT_UPDATE_ALLOWED "LyX 1.5.0svn"

!define ClassFileDir "${PRODUCT_SOURCEDIR}\Resources\tex"

;--------------------------------
; Make some of the information above available to NSIS.

Name "${PRODUCT_NAME}"
OutFile "${INSTALLER_EXE}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME} ${PRODUCT_VERSION}"

;--------------------------------
; Variables

Var LatexPath
Var EditorPath
Var PythonPath
Var PathPrefix
Var Answer
Var UserName
Var CreateDesktopIcon
Var StartmenuFolder
Var ProductRootKey
Var AppPre
Var AppSuff
Var AppPath
Var String
Var Search
Var Pointer
Var UserList
Var INSTDIR_NEW

;--------------------------------
; load some NSIS libraries
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"

!include "WordFunc.nsh"
!insertmacro WordReplace

; Set of various macros and functions
!include "LyXUtils.nsh"

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

!insertmacro MUI_LANGUAGE "English" # first language is the default language
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

;--------------------------------
; This hook function is called internally by NSIS on installer startup
Function .onInit

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
  
  ; Check where LyX is installed
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY_OLD}" "DisplayIcon"
  ${if} $0 == ""
   ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY_OLD}" "DisplayIcon"
   ${if} $0 == ""
    MessageBox MB_OK|MB_ICONSTOP "$(UpdateNotAllowed)"
    Abort
   ${endif}
  ${else}
   StrCpy $INSTDIR $0
   StrCpy $INSTDIR $INSTDIR -12 ; delete the string "\bin\lyx.exe" or "\bin\lyx.bat"
  ${endif}
  
  ; ascertain the registry root key of the LyX installation
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY_OLD}" "RootKey"
  ${if} $0 != ""
   SetShellVarContext all
   StrCpy $ProductRootKey "HKLM"
  ${else}
   ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY_OLD}" "RootKey"
   ${if} $0 == ""
     MessageBox MB_OK|MB_ICONSTOP "$(UnNotInRegistryLabel)"
     Abort
   ${else}
    SetShellVarContext current
    StrCpy $ProductRootKey "HKCU"
   ${endif}
  ${endif}
  
  ; This can be reset to "true" in section SecDesktop.
  StrCpy $CreateDesktopIcon "false"

  ; If the user does *not* have administrator privileges, abort
  StrCpy $Answer ""
  StrCpy $UserName ""
  !insertmacro IsUserAdmin $Answer $UserName ; macro from LyXUtils.nsh
  ${if} $Answer != "yes"

    MessageBox MB_OK|MB_ICONSTOP "$(NotAdmin)"
    Abort
  ${endif}
  
  ClearErrors
FunctionEnd

;--------------------------------

Function LaunchProduct
  Exec ${PRODUCT_BAT}
FunctionEnd

;--------------------------------

; The '-' makes the section invisible.
; Sections are entered in order, so the settings above are all
; available to SecInstallation
Section "-Installation actions" SecInstallation

  ; extract modified files
  Call UpdateModifiedFiles
  
  ; delete files
  Call DeleteFiles

  ; delete old start menu folder
  ReadRegStr $0 SHCTX "${PRODUCT_UNINST_KEY_OLD}" "StartMenu"
  RMDir /r $0
  ; delete desktop icon
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  
  ; delete old registry entries
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY_OLD}"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY_OLD}"
  DeleteRegKey HKCR "Applications\lyx.bat"
  
  ; determine the new name of the install location,
  ; only when the user has used the default path settings of the previous LyX-version
  StrCpy $String $INSTDIR
  StrCpy $Search "${PRODUCT_VERSION_OLD}"
  StrLen $3 $String
  Call StrPoint ; search the LaTeXPath for the phrase "${PRODUCT_VERSION_OLD}" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" ; if something was found
   IntOp $Pointer $Pointer - 1 ; jump before the first "\" of "\${PRODUCT_VERSION_OLD}"
   StrCpy $String $String "$Pointer" ; $String is now the part before "\${PRODUCT_VERSION_OLD}"
   ; rename the installation folder by copying LyX files
   StrCpy $INSTDIR_NEW "$String\LyX ${PRODUCT_VERSION}"
   CreateDirectory "$INSTDIR_NEW"
   CopyFiles "$INSTDIR\*.*" "$INSTDIR_NEW"
   ; delete the old folder
   RMDir /r $INSTDIR
   StrCpy $INSTDIR $INSTDIR_NEW
   ; read the PATH_PREFIX
   FileOpen $R1 "$INSTDIR\Resources\lyxrc.dist" r
   FileRead $R1 $PathPrefix
   ${WordReplace} $PathPrefix "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $PathPrefix
   FileClose $R1
   ; set the PATH_PREFIX according to the new folder
   FileOpen $R1 "$INSTDIR\Resources\lyxrc.dist" w
   FileSeek $R1 0 ; set file pointer to the beginning
   FileWrite $R1 '\path_prefix "$PathPrefix"$\r$\n' ; overwrite the existing path with the actual one
   FileClose $R1
  ${endif}
  
  ; register LyX
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "${PRODUCT_EXE}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "RootKey" "$ProductRootKey"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "LyX ${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "${PRODUCT_UNINSTALL_EXE}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "${PRODUCT_EXE}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "${PRODUCT_INFO_URL}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_ABOUT_URL}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "Publisher" "LyX Team"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "HelpLink" "${PRODUCT_HELP_LINK}"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" 0x00000001
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" 0x00000001
 
  ; create start menu entry  
  SetOutPath "$INSTDIR\bin"
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "${PRODUCT_UNINSTALL_EXE}"
  
  ; create desktop icon
  ${if} $CreateDesktopIcon == "true"
   SetOutPath "$INSTDIR\bin"
   CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  ${endif}

  ; test if Python is installed
  ; only use an existing python when it is version 2.5 because many Compaq and Dell PC are delivered
  ; with outdated Python interpreters
  ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.5\InstallPath" ""
  ${if} $PythonPath == ""
   StrCpy $PythonPath "$INSTDIR\bin"
  ${else}
   StrCpy $PythonPath $PythonPath -1 ; remove the "\" at the end
  ${endif}
  
  ; run LyX's configure script
  ; create a bat-file to start configure in a console window so that the user see the progress
  ; of the configuration and to have a signal when the configuration is ready to start LyX
  ; this is important when LyX is installed together with MiKTeX or when LyX is installed for the first
  ; time on a computer, because the installation of missing LaTeX-files required by LyX could last minutes
  ; a batch file is needed because simply calling ExecWait '"$PythonPath\python.exe" "$INSTDIR\Resources\configure.py"'
  ; creates the config files in $INSTDIR\bin
  StrCpy $1 $INSTDIR 2 ; get drive letter
  FileOpen $R1 "$INSTDIR\Resources\configLyX.bat" w
  FileWrite $R1 '$1$\r$\n\
  		 cd $INSTDIR\Resources\$\r$\n\
  		 "$PythonPath\python.exe" configure.py'
  FileClose $R1
  MessageBox MB_OK|MB_ICONINFORMATION "$(LatexConfigInfo)"
  ExecWait '"$INSTDIR\Resources\configLyX.bat"'
  ;Delete "$INSTDIR\Resources\configLyX.bat"

SectionEnd

; eof
