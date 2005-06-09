; Lyx for Windows, NSIS v2 series installer script

; File lyx_installer.nsi
; This file is part of LyX, the document processor.
; http://www.lyx.org/
; Licence details can be found in the file COPYING or copy at
; http://www.lyx.org/about/license.php3

; Author Angus Leeming
; Full author contact details are available in file CREDITS or copy at
; http://www.lyx.org/about/credits.php

; This script requires NSIS 2.06 and above
; http://nsis.sourceforge.net/
;--------------------------------

; Do a Cyclic Redundancy Check to make sure the installer
; was not corrupted by the download.
CRCCheck force

; Make the installer as small as possible.
SetCompressor lzma

;--------------------------------
; You should need to change only these macros...

!define PRODUCT_NAME "LyX"
!define PRODUCT_VERSION "1.3.6"
!define PRODUCT_LICENSE_FILE "..\..\..\..\COPYING"
!define PRODUCT_SOURCEDIR "J:\Programs\LyX"
!define PRODUCT_EXE "$INSTDIR\bin\lyx.exe"
!define PRODUCT_EXT ".lyx"
!define PRODUCT_MIME_TYPE "application/lyx"
!define PRODUCT_UNINSTALL_EXE "$INSTDIR\uninstall.exe"

!define INSTALLER_EXE "lyx_setup_136.exe"
!define INSTALLER_ICON "..\icons\lyx_32x32.ico"

; Replaced by HKLM or HKCU depending on SetShellVarContext.
!define PRODUCT_ROOT_KEY "SHCTX"

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\lyx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

;--------------------------------
; Make some of the information above available to NSIS.

Name "${PRODUCT_NAME}"
OutFile "${INSTALLER_EXE}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"

;--------------------------------
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "StrFunc.nsh"
!include "strtrim.nsh"
!include "download.nsh"

; Declare used functions
${StrLoc}
${StrRep}
${StrTrim}
${ReadDownloadValues}
${EnableBrowseControls}
${DownloadEnter}
${DownloadLeave}

; Grabbed from
; http://nsis.sourceforge.net/archive/viewpage.php?pageid=275
!include "is_user_admin.nsh"

; Grabbed from
; http://abiword.pchasm.org/source/cvs/abiword-cvs/abi/src/pkg/win/setup/NSISv2/abi_util_fileassoc.nsh
; Use the Abiword macros to help set up associations with the file extension.
; in the Registry.
!include "abi_util_fileassoc.nsh"

;--------------------------------
; Variables

Var MinSYSPath
Var DownloadMinSYS

Var PythonPath
Var DownloadPython

Var DoNotRequireMiKTeX
Var MiKTeXPath
Var DownloadMiKTeX

Var DoNotRequirePerl
Var PerlPath
Var DownloadPerl

Var DoNotRequireGhostscript
Var GhostscriptPath
Var DownloadGhostscript

Var DoNotRequireImageMagick
Var ImageMagickPath
Var DownloadImageMagick

Var DoNotInstallLyX
Var PathPrefix

Var CreateFileAssociations
Var CreateDesktopIcon
Var StartmenuFolder
Var ProductRootKey

;--------------------------------

; Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

!define MUI_ABORTWARNING
!define MUI_ICON "${INSTALLER_ICON}"
!define MUI_UNICON "${INSTALLER_ICON}"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

Page custom DownloadMinSYS DownloadMinSYS_LeaveFunction
Page custom DownloadPython DownloadPython_LeaveFunction
Page custom DownloadMiKTeX DownloadMiKTeX_LeaveFunction
Page custom DownloadPerl DownloadPerl_LeaveFunction
Page custom DownloadGhostscript DownloadGhostscript_LeaveFunction
Page custom DownloadImageMagick DownloadImageMagick_LeaveFunction
Page custom SummariseDownloads SummariseDownloads_LeaveFunction

; Show the license.
!insertmacro MUI_PAGE_LICENSE "${PRODUCT_LICENSE_FILE}"

; Specify the installation directory.
!insertmacro MUI_PAGE_DIRECTORY

; Define which components to install.
!insertmacro MUI_PAGE_COMPONENTS

; Specify where to install program shortcuts.
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $StartmenuFolder

; Watch the components being installed.
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TEXT "$(FinishPageMessage)"
!define MUI_FINISHPAGE_RUN_TEXT "$(FinishPageRun)"
!define MUI_FINISHPAGE_RUN "${PRODUCT_EXE}"
!insertmacro MUI_PAGE_FINISH

; The uninstaller.
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English" # first language is the default language
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "French"

!include "lyx_languages\english.nsh"
!include "lyx_languages\danish.nsh"
!include "lyx_languages\french.nsh"
!include "lyx_languages\german.nsh"
!include "lyx_languages\spanish.nsh"

LicenseData "$(LyXLicenseData)"

;--------------------------------
; Reserve Files

; These files should be inserted before other files in the data block
; Keep these lines before any File command
; Only for solid compression (by default, solid compression
; is enabled for BZIP2 and LZMA)
ReserveFile "ioDownload.ini"
ReserveFile "ioSummary.ini"
!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------

Section "!${PRODUCT_NAME}" SecCore
  SectionIn RO
SectionEnd

Section /o "$(SecAllUsersTitle)" SecAllUsers
  SetShellVarContext all
  StrCpy $ProductRootKey "HKLM"
SectionEnd

Section "$(SecFileAssocTitle)" SecFileAssoc
  StrCpy $CreateFileAssociations "true"
SectionEnd

Section "$(SecDesktopTitle)" SecDesktop
  StrCpy $CreateDesktopIcon "true"
SectionEnd

; The '-' makes the section invisible.
; Sections are entered in order, so the settings above are all
; available to SecInstallation
Section "-Installation actions" SecInstallation
  SetOverwrite off
  SetOutPath "$INSTDIR"
  File /r "${PRODUCT_SOURCEDIR}\Resources"
  File /r "${PRODUCT_SOURCEDIR}\bin"

  ${if} "$PathPrefix" != ""
    lyx_path_prefix::set_path_prefix "$INSTDIR\Resources\lyx\configure" "$PathPrefix"
    Pop $0
    ${if} $0 != 0
      MessageBox MB_OK "$(ModifyingConfigureFailed)"
    ${endif}
    lyx_path_prefix::run_configure "$INSTDIR\Resources\lyx\configure" "$PathPrefix"
    Pop $0
    ${if} $0 != 0
      MessageBox MB_OK "$(RunConfigureFailed)"
    ${endif}
  ${endif}

  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "${PRODUCT_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "RootKey" "$ProductRootKey"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "${PRODUCT_UNINSTALL_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "${PRODUCT_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"

  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${PRODUCT_NAME}.lnk" "${PRODUCT_EXE}"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "${PRODUCT_UNINSTALL_EXE}"

  ${if} $CreateDesktopIcon == "true"
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "${PRODUCT_EXE}"
  ${endif}

  ${if} $CreateFileAssociations == "true"
    ${CreateApplicationAssociation} \
      "${PRODUCT_NAME}" \
      "${PRODUCT_NAME}" \
      "${PRODUCT_NAME} Document" \
      "${PRODUCT_EXE},1" \
      "${PRODUCT_EXE}"

    ${CreateFileAssociation} "${PRODUCT_EXT}" "${PRODUCT_NAME}" "${PRODUCT_MIME_TYPE}"
  ${endif}

  WriteUninstaller "${PRODUCT_UNINSTALL_EXE}"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(SecCoreDescription)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAllUsers} "$(SecAllUsersDescription)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "$(SecFileAssocDescription)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

!define MUI_LANGDLL_ALWAYSSHOW

; This hook function is called internally by NSIS on installer startup
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "ioDownload.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "ioSummary.ini"

  ; Default settings
  ; These can be reset to "all" in section SecAllUsers.
  SetShellVarContext current
  StrCpy $ProductRootKey "HKCU"

  ; This can be reset to "true" in section SecDesktop.
  StrCpy $CreateDesktopIcon "false"
  StrCpy $CreateFileAssociations "false"

  ; If the user does *not* have administrator privileges,
  ; then make section SecAllUsers readonly.
  Call IsUserAdmin
  Pop $0
  ${if} $0 == "true"
    !define ENABLE 0x00000001
    SectionGetFlags ${SecAllUsers} $0
    IntOp $0 $0 | ${ENABLE}
    SectionSetFlags ${SecAllUsers} $0
    !undef ENABLE
  ${else}
    !define READ_ONLY 0x00000010
    SectionGetFlags ${SecAllUsers} $0
    IntOp $0 $0 | ${READ_ONLY}
    SectionSetFlags ${SecAllUsers} $0
    !undef READ_ONLY
  ${endif}

  ClearErrors
FunctionEnd

;--------------------------------

Function DownloadMinSYS
  StrCpy $MinSYSPath ""
  StrCpy $DownloadMinSYS "0"

  ; Search the registry for the MinSYS uninstaller.
  ; If successful, put its location in $2.
  StrCpy $3 "Software\Microsoft\Windows\CurrentVersion\Uninstall"
  StrCpy $2 ""
  StrCpy $0 0
  loop:
    EnumRegKey $1 HKLM "$3" $0
    ${if} $1 == ""
      Goto done
    ${endif}

    ${StrLoc} $2 "$1" "MSYS-1.0" "<"
    ${if} $2 > 0
      StrCpy $2 "$3\$1"
      Goto done
    ${else}
      StrCpy $2 ""
    ${endif}
    IntOp $0 $0 + 1
    Goto loop
  done:

  ${DownloadEnter} \
      $MinSYSPath "$2" "Inno Setup: App Path" \
      "" "\bin" \
      0 \
      "$(MinSYSDownloadLabel)" \
      "$(MinSYSFolderLabel)" \
      "$(MinSYSHeader)" \
      "$(MinSYSDescription)"
FunctionEnd

Function DownloadMinSYS_LeaveFunction
  ${DownloadLeave} \
      $0 \
      $DownloadMinSYS \
      $MinSYSPath \
      "http://sourceforge.net/project/showfiles.php?group_id=2435&package_id=82721&release_id=158803" \
      "$(EnterMinSYSFolder)" \
      "\sh.exe" \
      "$(InvalidMinSYSFolder)"
FunctionEnd

;--------------------------------

Function DownloadPython
  StrCpy $PythonPath ""
  StrCpy $DownloadPython "0"

  ${DownloadEnter} \
      $PythonPath "Software\Microsoft\Windows\CurrentVersion\App Paths\Python.exe" "" \
      "\Python.exe" "" \
      0 \
      "$(PythonDownloadLabel)" \
      "$(PythonFolderLabel)" \
      "$(PythonHeader)" \
      "$(PythonDescription)"
FunctionEnd

Function DownloadPython_LeaveFunction
  ${DownloadLeave} \
      $0 \
      $DownloadPython \
      $PythonPath \
      "http://www.python.org/download/" \
      "$(EnterPythonFolder)" \
      "\Python.exe" \
      "$(InvalidPythonFolder)"
FunctionEnd

;--------------------------------

Function DownloadMiKTeX
  StrCpy $DoNotRequireMiKTeX "1"
  StrCpy $MiKTeXPath ""
  StrCpy $DownloadMiKTeX "0"

  ${DownloadEnter} \
      $MiKTeXPath "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Install Root" \
      "" "\miktex\bin" \
      1 \
      "$(MiKTeXDownloadLabel)" \
      "$(MiKTeXFolderLabel)" \
      "$(MiKTeXHeader)" \
      "$(MiKTeXDescription)"
FunctionEnd

Function DownloadMiKTeX_LeaveFunction
  ${DownloadLeave} \
      $DoNotRequireMiKTeX \
      $DownloadMiKTeX \
      $MiKTeXPath \
      "http://www.miktex.org/setup.html" \
      "$(EnterMiKTeXFolder)" \
      "\latex.exe" \
      "$(InvalidMiKTeXFolder)"
FunctionEnd

;--------------------------------

Function DownloadPerl
  StrCpy $DoNotRequirePerl "1"
  StrCpy $PerlPath ""
  StrCpy $DownloadPerl "1"

  ${DownloadEnter} \
      $PerlPath "Software\Perl" BinDir \
      "\perl.exe" "" \
      1 \
      "$(PerlDownloadLabel)" \
      "$(PerlFolderLabel)" \
      "$(PerlHeader)" \
      "$(PerlDescription)"
FunctionEnd

Function DownloadPerl_LeaveFunction
  ${DownloadLeave} \
      $DoNotRequirePerl \
      $DownloadPerl \
      $PerlPath \
      "http://www.activestate.com/Products/ActivePerl/" \
      "$(EnterPerlFolder)" \
      "\perl.exe" \
      "$(InvalidPerlFolder)"
FunctionEnd

;--------------------------------

Function DownloadGhostscript
  StrCpy $DoNotRequireGhostscript "1"
  StrCpy $GhostscriptPath ""
  StrCpy $DownloadGhostscript "0"

  ; Find which version of ghostscript, if any, is installed.
  EnumRegKey $1 HKLM "Software\AFPL Ghostscript" 0
  ${if} $1 != ""
    StrCpy $0 "Software\AFPL Ghostscript\$1"
  ${else}
    StrCpy $0 ""
  ${endif}

  ${DownloadEnter} \
      $GhostscriptPath "$0" "GS_DLL" \
      "\gsdll32.dll" "" \
      1 \
      "$(GhostscriptDownloadLabel)" \
      "$(GhostscriptFolderLabel)" \
      "$(GhostscriptHeader)" \
      "$(GhostscriptDescription)"
FunctionEnd

Function DownloadGhostscript_LeaveFunction
  ${DownloadLeave} \
      $DoNotRequireGhostscript \
      $DownloadGhostscript \
      $GhostscriptPath \
      "http://www.cs.wisc.edu/~ghost/doc/AFPL/index.htm" \
      "$(EnterGhostscriptFolder)" \
      "\gswin32c.exe" \
      "$(InvalidGhostscriptFolder)"
FunctionEnd

;--------------------------------

Function DownloadImageMagick
  StrCpy $DoNotRequireImageMagick "1"
  StrCpy $ImageMagickPath ""
  StrCpy $DownloadImageMagick "0"

  ${DownloadEnter} \
      $ImageMagickPath "Software\ImageMagick\Current" "BinPath" \
      "" "" \
      1 \
      "$(ImageMagickDownloadLabel)" \
      "$(ImageMagickFolderLabel)" \
      "$(ImageMagickHeader)" \
      "$(ImageMagickDescription)"
FunctionEnd

Function DownloadImageMagick_LeaveFunction
  ${DownloadLeave} \
      $DoNotRequireImageMagick \
      $DownloadImageMagick \
      $ImageMagickPath \
      "http://www.imagemagick.org/script/binary-releases.php" \
      "$(EnterImageMagickFolder)" \
      "\convert.exe" \
      "$(InvalidImageMagickFolder)"
FunctionEnd

;--------------------------------

Function SummariseDownloads

  StrCpy $PathPrefix ""
  ${if} $MinSYSPath != ""
    StrCpy $PathPrefix "$PathPrefix;$MinSYSPath"
  ${endif}
  ${if} $PythonPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PythonPath"
  ${endif}
  ${if} $MiKTeXPath != ""
    StrCpy $PathPrefix "$PathPrefix;$MiKTeXPath"
  ${endif}
  ${if} $PerlPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PerlPath"
  ${endif}
  ${if} $GhostscriptPath != ""
    StrCpy $PathPrefix "$PathPrefix;$GhostscriptPath"
  ${endif}
  ${if} $ImageMagickPath != ""
    StrCpy $PathPrefix "$PathPrefix;$ImageMagickPath"
  ${endif}
  ; Remove the leading ';'
  StrCpy $PathPrefix "$PathPrefix" "" 1

  IntOp $DoNotInstallLyX $DownloadMinSYS + $DownloadPython
  IntOp $DoNotInstallLyX $DoNotInstallLyX + $DownloadMiKTeX
  IntOp $DoNotInstallLyX $DoNotInstallLyX + $DownloadPerl
  IntOp $DoNotInstallLyX $DoNotInstallLyX + $DownloadGhostscript
  IntOp $DoNotInstallLyX $DoNotInstallLyX + $DownloadImageMagick

  ${if} "$DoNotInstallLyX" == 1
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSummary.ini" "Field 1" "Text" "$(SummaryPleaseInstall)"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSummary.ini" "Field 2" "Text" ""
  ${else}
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSummary.ini" "Field 1" "Text" "$(SummaryPathPrefix)"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSummary.ini" "Field 2" "Text" "$PathPrefix"
  ${endif}

  !insertmacro MUI_HEADER_TEXT "$(SummaryTitle)" ""
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "ioSummary.ini"
FunctionEnd

Function SummariseDownloads_LeaveFunction
  ${if} "$DoNotInstallLyX" == 1
    Quit
  ${endif}
FunctionEnd

;--------------------------------
; The Uninstaller

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

  ; Ascertain whether the user has sufficient privileges to uninstall.
  SetShellVarContext current

  ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY}" "RootKey"
  ${if} $0 == ""
    ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "RootKey"
    ${if} $0 == ""
      MessageBox MB_OK "$(UnNotInRegistryLabel)"
    ${endif}
  ${endif}

  ${if} $0 == "HKLM"
    Call un.IsUserAdmin
    Pop $0
    ${if} $0 == "true"
      SetShellVarContext all
    ${else}
      MessageBox MB_OK "$(UnNotAdminLabel)"
      Abort
    ${endif}
  ${endif}

  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" IDYES +2
  Abort
FunctionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(UnRemoveSuccessLabel)"
FunctionEnd


Section Uninstall
  RMDir /r $INSTDIR

  ReadRegStr $0 ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "StartMenu"
  RMDir /r "$0"

  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  DeleteRegKey "HKCU" "${PRODUCT_UNINST_KEY}\Installer Language"
  DeleteRegKey ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  ${RemoveFileAssociation} "${PRODUCT_EXT}" "${PRODUCT_NAME}"

  SetAutoClose true
SectionEnd

; eof
