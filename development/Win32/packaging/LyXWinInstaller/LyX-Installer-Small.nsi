; Lyx for Windows, NSIS v2 series installer script

; File LyXWinInstaller.nsi
; This file is part of LyX, the document processor.
; http://www.lyx.org/
; Licence details can be found in the file COPYING or copy at
; http://www.lyx.org/about/license.php3

; Author Uwe Stöhr based on the work of Angus Leeming and Joost Verburg
; Full author contact details are available in file CREDITS or copy at
; http://www.lyx.org/about/credits.php

; This script requires NSIS 2.22 and newer
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
!define PRODUCT_VERSION "1.5svn-23-01-2007"
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

BrandingText "LyXWinInstaller v3.05 - Small"
!define INSTALLER_VERSION "Small"
!define INSTALLER_EXE "LyXWin150svnSmall-3-05.exe"
!define INSTALLER2_EXE "LyXWin150svnComplete-3-05.exe" ; to check later if this installer version is running at the same time
!define VERSION_BITMAP "${PRODUCT_DIR}\icons\lyx_logo_vert${PRODUCT_VERSION_SHORT}.bmp"

; Replaced by HKLM or HKCU depending on SetShellVarContext.
!define PRODUCT_ROOT_KEY "SHCTX"

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\lyx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\LyX150svn"

!define ClassFileDir "${PRODUCT_SOURCEDIR}\Resources\tex"

!define ImageMagickVersion "6.3.2"
!define ImageMagickDir "$INSTDIR\etc\ImageMagick" ; for some odd reason the ImageMagick folder may not be a subfolder of $INSTDIR\bin!
!define GhostscriptDir "$INSTDIR\etc\Ghostscript"
!define GhostscriptVersion "8.54"
!define AiksaurusDir "$APPDATA\Aiksaurus"
!define AiksaurusInstall "external\Aiksaurus"
!define AspellDir "$APPDATA\Aspell"
!define AspellDisplay "Aspell 0.60.4 Data"
!define AspellInstall "external\Aspell"
!define AspellUninstall "Uninstall-AspellData.exe"
!define AspellDictPath "${AspellDir}\Dictionaries"
!define AspellPersonalPath "${AspellDir}\Personal"
!define AspellLocationExact "http://developer.berlios.de/project/showfiles.php?group_id=5117&release_id=9651"
!define AspellLocation "http://developer.berlios.de/projects/lyxwininstall/"
!define JabRefVersion "JabRef not included"
!define DVIPostFileDir "${PRODUCT_SOURCEDIR}\external\dvipost"
!define MiKTeXRepo "ftp://ftp.tu-chemnitz.de/pub/tex/systems/win32/miktex/tm/packages/"
!define MiKTeXConfigFolder "MiKTeX\2.5\miktex\config"
!define MiKTeXDeliveredVersion "MiKTeX not included"

;--------------------------------
; Make some of the information above available to NSIS.

Name "${PRODUCT_NAME}"
OutFile "${INSTALLER_EXE}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME} ${PRODUCT_VERSION}"

;--------------------------------
; Variables

Var LatexPath
Var PythonPath
Var DelPythonFiles
Var GhostscriptPath
Var ImageMagickPath
Var AiksaurusPath
Var AspellPath
Var AspellInstallYes
Var AspellBaseReg
Var AspellMessage
Var PDFVPath
Var Acrobat
Var PSVPath
Var EditorPath
Var ImageEditorPath
Var BibTeXEditorPath
Var JabRefInstalled
Var PathPrefix
Var Answer
Var UserName
Var LangName
Var LangNameSys
Var LangCode
Var LangCodeSys
Var LangEncoding
Var LangSysEncoding
Var DictCode
Var MissedProg
Var LaTeXName
Var MiKTeXVersion
Var MiKTeXInstalled
Var MiKTeXUser
Var CreateFileAssociations
Var CreateDesktopIcon
Var StartmenuFolder
Var ProductRootKey
Var AppPre
Var AppSuff
Var AppPath
Var State
Var String
Var Search
Var Pointer
Var UserList
Var RunNumber

;--------------------------------
; load some NSIS libraries
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"
;!include "TextFunc.nsh"
;!insertmacro LineFind

; Set of various macros and functions
!include "LyXUtils.nsh"

; Function to check if needed programs are missing or not
!include "MissingPrograms.nsh"

; Function for page to manually select LaTeX's installation folder
!include "LaTeXFolder.nsh"

; Functions for page to set installer language
; and LyX's menu language
!include "LanguageSettings.nsh"

; Function for page to install Aspell dictionaries
!include "Aspell.nsh"

; Use the Abiword macros to help set up associations with the file extension in the Registry.
; Grabbed from
; http://abiword.pchasm.org/source/cvs/abiword-cvs/abi/src/pkg/win/setup/NSISv2/abi_util_fileassoc.nsh
!include "abi_util_fileassoc.nsh"

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

; Specify the installation directory.
!insertmacro MUI_PAGE_DIRECTORY

; Specify LyX's menu language.
Page custom SelectMenuLanguage SelectMenuLanguage_LeaveFunction

; Define which components to install.
!insertmacro MUI_PAGE_COMPONENTS

; Specify where to install program shortcuts.
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "LyX ${PRODUCT_VERSION}"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $StartmenuFolder

; Select latex.exe manually
Page custom LatexFolder LatexFolder_LeaveFunction

; Check for needed programs
Page custom MissingProgramsPage MissingProgramsPage_LeaveFunction

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
ReserveFile "io_ui_language.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
; Installer sections

Section "!${PRODUCT_NAME}" SecCore
  SectionIn RO
SectionEnd
Section "$(SecAllUsersTitle)" SecAllUsers
  SetShellVarContext all
  StrCpy $ProductRootKey "HKLM"
SectionEnd
Section "$(SecFileAssocTitle)" SecFileAssoc
  StrCpy $CreateFileAssociations "true"
SectionEnd
Section "$(SecDesktopTitle)" SecDesktop
  StrCpy $CreateDesktopIcon "true"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(SecCoreDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecAllUsers} "$(SecAllUsersDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "$(SecFileAssocDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!include "InstallActions-small.nsh"

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

  ; Check if LyX is already installed
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "Publisher"
  ${if} $0 != ""
   MessageBox MB_OK|MB_ICONSTOP "$(StillInstalled)"
   Abort
  ${endif}

  ; Default settings
  ; These can be reset to "all" in section SecAllUsers.
  SetShellVarContext current
  StrCpy $ProductRootKey "HKCU"

  ; This can be reset to "true" in section SecDesktop.
  StrCpy $CreateDesktopIcon "false"
  StrCpy $CreateFileAssociations "false"

  ; If the user does *not* have administrator privileges, abort
  StrCpy $Answer ""
  StrCpy $UserName ""
  !insertmacro IsUserAdmin $Answer $UserName ; macro from LyXUtils.nsh
  ${if} $Answer == "yes"
    !define ENABLE 0x00000001
    SectionGetFlags ${SecAllUsers} $0
    IntOp $0 $0 | ${ENABLE}
    SectionSetFlags ${SecAllUsers} $0
    !undef ENABLE
  ${else}
    MessageBox MB_OK|MB_ICONSTOP "$(NotAdmin)"
    Abort
  ${endif}

  ; check which programs are installed or not
  Call MissingPrograms ; function from MissingPrograms.nsh

  ; used later in the function LatexFolder
  StrCpy $State "0"

  ClearErrors
FunctionEnd

;--------------------------------

Function LaunchProduct
  Exec ${PRODUCT_BAT}
FunctionEnd

;--------------------------------

;Function DeleteLines
;	StrCpy $0 SkipWrite
;	Push $0
;FunctionEnd

;--------------------------------
; The Uninstaller

Function un.onInit

  ; Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}

  ; Ascertain whether the user has sufficient privileges to uninstall.
  SetShellVarContext current

  ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY}" "RootKey"
  ${if} $0 == ""
    ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "RootKey"
    ${if} $0 == ""
      MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)"
    ${endif}
  ${endif}

  ; If the user does *not* have administrator privileges, abort
  StrCpy $Answer ""
  !insertmacro IsUserAdmin $Answer $UserName ; macro from LyXUtils.nsh
  ${if} $Answer == "yes"
    SetShellVarContext all
  ${else}
    MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)"
    Abort
  ${endif}

  ; Macro to investigate name of LyX's preferences folders to be able remove them
  !insertmacro UnAppPreSuff $AppPre $AppSuff ; macro from LyXUtils.nsh

  ; test if Aspell was installed together with LyX
  ReadRegStr $0 HKLM "Software\Aspell" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes"
   SectionSetText 2 "Aspell" ; names the corersponding uninstaller section (has the index "2" as it is the third section in Uninstall.nsh)
   StrCpy $AspellInstallYes "Aspell"
  ${else}
   SectionSetText 2 "" ; hides the corresponding uninstaller section
  ${endif}

  ; test if MiKTeX was installed together with LyX
  ReadRegStr $0 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${if} $0 == "Yes"
   SectionSetText 3 "MiKTeX" ; names the corersponding uninstaller section
   StrCpy $MiKTeXInstalled "MiKTeX"
  ${else}
   SectionSetText 3 "" ; hides the corresponding uninstaller section
  ${endif}

  ; ignore JabRef because this could only be installed with the complete installer version
   SectionSetText 4 "" ; hides the corresponding uninstaller section
   StrCpy $JabRefInstalled ""

  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" IDYES +2
  Abort

FunctionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(UnRemoveSuccessLabel)"
  
FunctionEnd

;----------------------
;Installer sections
!include "Uninstall.nsh"

; eof
