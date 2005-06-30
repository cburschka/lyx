; Lyx for Windows, NSIS v2 series installer script

; File lyx_installer.nsi
; This file is part of LyX, the document processor.
; http://www.lyx.org/
; Licence details can be found in the file COPYING or copy at
; http://www.lyx.org/about/license.php3

; Author Angus Leeming
; Author Uwe Stöhr
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
!define PRODUCT_BAT "$INSTDIR\bin\lyx.bat"
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
!include "lyx_utils.nsh"

; Grabbed from
; http://nsis.sourceforge.net/archive/viewpage.php?pageid=275
!include "is_user_admin.nsh"

; Grabbed from
; http://abiword.pchasm.org/source/cvs/abiword-cvs/abi/src/pkg/win/setup/NSISv2/abi_util_fileassoc.nsh
; Use the Abiword macros to help set up associations with the file extension.
; in the Registry.
!include "abi_util_fileassoc.nsh"

;--------------------------------
; Declare used functions

${StrStrAdv}
${StrLoc}
${StrNSISToIO}
${StrRep}
${StrTok}
${StrTrim}
${StrLTrim}
${StrRTrim}
${StrRTrimChar}
${ReadDownloadValues}
${EnableBrowseControls}
${SearchRegistry}
${DownloadEnter}
${DownloadLeave}

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

Var PDFViewerPath
Var PDFViewerProg

Var PSViewerPath
Var PSViewerProg

Var DoNotInstallLyX
Var PathPrefix

Var CreateFileAssociations
Var CreateDesktopIcon
Var StartmenuFolder
Var ProductRootKey

Var LangName
Var LangCode

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

; Specify LyX's menu language.
Page custom SelectMenuLanguage SelectMenuLanguage_LeaveFunction

; Define which components to install.
!insertmacro MUI_PAGE_COMPONENTS

; Specify where to install program shortcuts.
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $StartmenuFolder

; Watch the components being installed.
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_TEXT "$(FinishPageMessage)"
!define MUI_FINISHPAGE_RUN_TEXT "$(FinishPageRun)"
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchProduct"
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
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Swedish"

!include "lyx_languages\english.nsh"
!include "lyx_languages\danish.nsh"
!include "lyx_languages\dutch.nsh"
!include "lyx_languages\french.nsh"
!include "lyx_languages\german.nsh"
!include "lyx_languages\italian.nsh"
!include "lyx_languages\spanish.nsh"
!include "lyx_languages\swedish.nsh"

LicenseData "$(LyXLicenseData)"

;--------------------------------
; Reserve Files

; These files should be inserted before other files in the data block
; Keep these lines before any File command
; Only for solid compression (by default, solid compression
; is enabled for BZIP2 and LZMA)
ReserveFile "io_download.ini"
ReserveFile "io_summary.ini"
!insertmacro MUI_RESERVEFILE_LANGDLL
ReserveFile "io_ui_language.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

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
  SetOverwrite on
  SetOutPath "$INSTDIR"
  File /r "${PRODUCT_SOURCEDIR}\Resources"
  File /r "${PRODUCT_SOURCEDIR}\bin"

  ${if} "$PathPrefix" != ""
    lyx_configure::set_path_prefix "$INSTDIR\Resources\lyx\configure" "$PathPrefix"
    Pop $0
    ${if} $0 != 0
      MessageBox MB_OK "$(ModifyingConfigureFailed)"
    ${endif}
  ${endif}

  lyx_configure::create_bat_files "$INSTDIR\bin" "$LangCode"
  Pop $0
  ${if} $0 != 0
    MessageBox MB_OK "$(CreateCmdFilesFailed)"
  ${endif}

  lyx_configure::run_configure "$INSTDIR\Resources\lyx\configure" "$PathPrefix"
  Pop $0
  ${if} $0 != 0
    MessageBox MB_OK "$(RunConfigureFailed)"
  ${endif}

  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "${PRODUCT_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "RootKey" "$ProductRootKey"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "${PRODUCT_UNINSTALL_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "${PRODUCT_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"

  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "${PRODUCT_UNINSTALL_EXE}"

  ${if} $CreateDesktopIcon == "true"
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  ${endif}

  ${if} $CreateFileAssociations == "true"
    ${CreateApplicationAssociation} \
      "${PRODUCT_NAME}" \
      "${PRODUCT_NAME}" \
      "${PRODUCT_NAME} Document" \
      "${PRODUCT_EXE},1" \
      "${PRODUCT_BAT}"

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

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "io_download.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "io_summary.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "io_ui_language.ini"

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

  Call SearchMinSYS
  Call SearchPython
  Call SearchMiKTeX
  Call SearchPerl
  Call SearchGhostscript
  Call SearchImageMagick
  Call SearchPDFViewer
  Call SearchPSViewer

  ClearErrors
FunctionEnd

;--------------------------------

Function LaunchProduct
  lyx_configure::set_env LANG $LangCode
  Exec ${PRODUCT_EXE}
FunctionEnd

;--------------------------------

; Sets the value of the global $MinSYSPath variable.
Function SearchMinSYS
  ; This function manipulates the registers $0-$3,
  ; so push their current content onto the stack.
  Push $0
  Push $1
  Push $2
  Push $3

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

  ${SearchRegistry} \
      $MinSYSPath \
      "$2" \
      "Inno Setup: App Path" \
      "" \
      "\bin"

  ; Return the $0, $1, $2 and $2 registers to their original state
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd

Function DownloadMinSYS
  StrCpy $DownloadMinSYS "0"

  ${DownloadEnter} \
      $MinSYSPath \
      0 \
      "$(MinSYSDownloadLabel)" \
      "$(MinSYSFolderLabel)" \
      "$(MinSYSHeader)" \
      "$(MinSYSDescription)"
FunctionEnd

Function DownloadMinSYS_LeaveFunction
  ; This function manipulates the $0 register
  ; so push its current content onto the stack.
  Push $0

  ${DownloadLeave} \
      $0 \
      $DownloadMinSYS \
      $MinSYSPath \
      "http://sourceforge.net/project/showfiles.php?group_id=2435&package_id=82721&release_id=158803" \
      "$(EnterMinSYSFolder)" \
      "sh.exe" \
      "$(InvalidMinSYSFolder)"

  ; Return the $0 register to its original state
  Pop $0
FunctionEnd

;--------------------------------

; Sets the value of the global $PythonPath variable.
Function SearchPython
  ${SearchRegistry} \
      $PythonPath \
      "Software\Microsoft\Windows\CurrentVersion\App Paths\Python.exe" \
      "" \
      "\Python.exe" \
      ""
FunctionEnd

Function DownloadPython
  StrCpy $DownloadPython "0"

  ${DownloadEnter} \
      $PythonPath \
      0 \
      "$(PythonDownloadLabel)" \
      "$(PythonFolderLabel)" \
      "$(PythonHeader)" \
      "$(PythonDescription)"
FunctionEnd

Function DownloadPython_LeaveFunction
  ; This function manipulates the $0 register
  ; so push its current content onto the stack.
  Push $0

  ${DownloadLeave} \
      $0 \
      $DownloadPython \
      $PythonPath \
      "http://www.python.org/download/" \
      "$(EnterPythonFolder)" \
      "Python.exe" \
      "$(InvalidPythonFolder)"

  ; Return the $0 register to its original state
  Pop $0
FunctionEnd

;--------------------------------

; Sets the value of the global $MiKTeXPath variable.
Function SearchMiKTeX
  ${SearchRegistry} \
      $MiKTeXPath \
      "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" \
      "Install Root" \
      "" \
      "\miktex\bin"
FunctionEnd

Function DownloadMiKTeX
  StrCpy $DoNotRequireMiKTeX "1"
  StrCpy $DownloadMiKTeX "0"

  ${DownloadEnter} \
      $MiKTeXPath \
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
      "latex.exe" \
      "$(InvalidMiKTeXFolder)"
FunctionEnd

;--------------------------------

; Sets the value of the global $PerlPath variable.
Function SearchPerl
  ${SearchRegistry} \
      $PerlPath \
      "Software\Perl" \
      BinDir \
      "\perl.exe" \
      ""
FunctionEnd

Function DownloadPerl
  StrCpy $DoNotRequirePerl "1"
  StrCpy $DownloadPerl "1"

  ${DownloadEnter} \
      $PerlPath \
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
      "perl.exe" \
      "$(InvalidPerlFolder)"
FunctionEnd

;--------------------------------

; Sets the value of the global $GhostscriptPath variable.
Function SearchGhostscript
  ; This function manipulates the $0 and $1 registers,
  ; so push their current content onto the stack.
  Push $0
  Push $1

  ; Find which version of ghostscript, if any, is installed.
  ; Store this value in $0.
  StrCpy $0 ""
  EnumRegKey $1 HKLM "Software\AFPL Ghostscript" 0
  ${if} $1 != ""
    StrCpy $0 "Software\AFPL Ghostscript\$1"
  ${else}
    EnumRegKey $1 HKLM "Software\GPL Ghostscript" 0
    ${if} $1 != ""
      StrCpy $0 "Software\GPL Ghostscript\$1"
    ${endif}
  ${endif}

  ${SearchRegistry} \
      $GhostscriptPath \
      "$0" \
      "GS_DLL" \
      "\gsdll32.dll" \
      ""

  ; Return the $0 and $1 registers to their original states
  Pop $1
  Pop $0
FunctionEnd

Function DownloadGhostscript
  StrCpy $DoNotRequireGhostscript "1"
  StrCpy $DownloadGhostscript "0"

  ; Find which version of ghostscript, if any, is installed.
  EnumRegKey $1 HKLM "Software\AFPL Ghostscript" 0
  ${if} $1 == ""
   EnumRegKey $1 HKLM "Software\GPL Ghostscript" 0
   StrCpy $2 "True"
  ${endif}
  ${if} $1 != ""
   ${if} $2 == "True"
    StrCpy $0 "Software\GPL Ghostscript\$1"
   ${else}
    StrCpy $0 "Software\AFPL Ghostscript\$1"
   ${endif}
  ${else}
    StrCpy $0 ""
  ${endif}

  ${DownloadEnter} \
      $GhostscriptPath \
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
      "gswin32c.exe" \
      "$(InvalidGhostscriptFolder)"
FunctionEnd

;--------------------------------

; Sets the value of the global $ImageMagickPath variable.
Function SearchImageMagick
  ${SearchRegistry} \
      $ImageMagickPath \
      "Software\ImageMagick\Current" \
      "BinPath" \
      "" \
      ""
FunctionEnd

Function DownloadImageMagick
  StrCpy $DoNotRequireImageMagick "1"
  StrCpy $DownloadImageMagick "0"

  ${DownloadEnter} \
      $ImageMagickPath \
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
      "convert.exe" \
      "$(InvalidImageMagickFolder)"
FunctionEnd

;--------------------------------

; Sets the value of the global $PDFViewerPath and $PDFViewerProg variables.
Function SearchPDFViewer
  StrCpy $PDFViewerPath ""
  !insertmacro GetFileExtProg $PDFViewerPath $PDFViewerProg ".pdf" "a"
FunctionEnd

;--------------------------------

Function SearchPSViewer
  ; This function manipulates the $0 and $1 registers,
  ; so push their current content onto the stack.
  Push $0
  Push $1

  StrCpy $PSViewerPath ""
  StrCpy $0 ""
  StrCpy $1 ""
  !insertmacro GetFileExtProg $PSViewerPath $PSViewerProg ".ps" "a"
  ${if} $PSViewerPath != ""
    StrCpy $0 $PSViewerPath
    StrCpy $0 $0 "" -8
  ${endif}
  ${if} $0 == "Distillr"
    !insertmacro GetFileExtProg $0 $1 ".ps" "b"
    ${if} $1 != ""
      StrCpy $PSViewerPath $0
      StrCpy $PSViewerProg $1
    ${endif}
  ${endif}

  ; Failed to find anything that way. Try another.
  ${if} $PSViewerPath == ""
    ReadRegStr $PSViewerProg HKCR "psfile\shell\open\command" ""
    ; Extract the first quoted word.
    ${StrTok} $0 "$PSViewerProg" '"' '1' '0'
    ${if} $0 != ""
      StrCpy $PSViewerProg $0
    ${endif}

    ${StrTrim} $PSViewerProg "$PSViewerProg"
    ; Split into <path,exe> pair
    ${StrStrAdv} $PSViewerPath $PSViewerProg "\" "<" "<" "0" "0" "0"
    ${StrStrAdv} $PSViewerProg $PSViewerProg "\" "<" ">" "0" "0" "0"
  ${endif}

  ; Return the $0 and $1 registers to their original states
  Pop $1
  Pop $0
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
    !insertmacro MUI_INSTALLOPTIONS_WRITE "io_summary.ini" "Field 1" "Text" "$(SummaryPleaseInstall)"
  ${else}
    ${StrNSISToIO} $0 '$PathPrefix'
    ${StrRep} $0 "$0" ";" "\r\n"
    StrCpy $0 "$(SummaryPathPrefix)\r\n\r\n$0"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "io_summary.ini" "Field 1" "Text" "$0"
  ${endif}

  !insertmacro MUI_HEADER_TEXT "$(SummaryTitle)" ""
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "io_summary.ini"
FunctionEnd

Function SummariseDownloads_LeaveFunction
  ${if} "$DoNotInstallLyX" == 1
    Quit
  ${endif}
FunctionEnd

;--------------------------------

Function SelectMenuLanguage
  StrCpy $LangName ""

  ;tranlate NSIS's language code to the language name; macro from lyx_utils.nsh
  !insertmacro TranslateLangCode $LangName $Language

  !insertmacro MUI_INSTALLOPTIONS_WRITE "io_ui_language.ini" "Field 2" "State" "$LangName"

  !insertmacro MUI_HEADER_TEXT "$(UILangageTitle)" "$(UILangageDescription)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "io_ui_language.ini"
FunctionEnd

;--------------------------------

Function SelectMenuLanguage_LeaveFunction
  !insertmacro MUI_INSTALLOPTIONS_READ $LangName "io_ui_language.ini" "Field 2" "State"

  ;Get the language code; macro from lyx_utils.nsh
  StrCpy $LangCode ""
  !insertmacro GetLangCode $LangCode $LangName
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
