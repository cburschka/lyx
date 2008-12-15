# Lyx for Windows, NSIS v2 series installer script

# File LyXInstaller-update.nsi
# This file is part of LyX, the document processor.
# http://www.lyx.org/
# Licence details can be found in the file COPYING or copy at
# http://www.lyx.org/about/license.php3

# Author Uwe Stöhr
# Full author contact details are available in file CREDITS or copy at
# http://www.lyx.org/about/credits.php

# This script requires NSIS 2.30 or newer
# http://nsis.sourceforge.net/
#--------------------------------

# Do a Cyclic Redundancy Check to make sure the installer
# was not corrupted by the download.
CRCCheck force

# Make the installer as small as possible.
SetCompressor lzma

#--------------------------------
# You should need to change only these macros...

!define INSTALLER_VERSION "Update"
!define INSTALLER2_VERSION "Small"
!define INSTALLER3_VERSION "Complete"
!define INSTALLER_TYPE "Update"

# load the settings
!include "Settings.nsh"

#--------------------------------
# variables only used in this installer version

Var INSTDIR_NEW
Var INSTDIR_OLD
Var AppPath
# Variables used by all installer versions
!include "Variables.nsh"

#--------------------------------
# load some NSIS libraries
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"
!include "TextFunc.nsh"
!include "WinVer.nsh"
!insertmacro LineFind
!include "WordFunc.nsh"
!insertmacro WordReplace

# Set of various macros and functions
!include "LyXUtils.nsh"

# list with modified files
!include "Updated.nsh"

# list with deleted files
!include "Deleted.nsh"

# Function for page to install Aspell dictionaries
!include "Aspell.nsh"

#--------------------------------

# Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${PRODUCT_DIR}\icons\lyx_logo_hi.bmp"
!define MUI_ICON "${PRODUCT_DIR}\icons\lyx.ico"
!define MUI_UNICON "${PRODUCT_DIR}\icons\lyx.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${VERSION_BITMAP}"

# Welcome page
!define MUI_WELCOMEPAGE_TEXT "$(WelcomePageText)"
!insertmacro MUI_PAGE_WELCOME

# Show the license.
!insertmacro MUI_PAGE_LICENSE "${PRODUCT_LICENSE_FILE}"

# Define which components to install.
!insertmacro MUI_PAGE_COMPONENTS

# Specify where to install program shortcuts.
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "LyX ${PRODUCT_VERSION}"
!insertmacro MUI_PAGE_STARTMENU ${PRODUCT_NAME} $StartmenuFolder

# Watch the components being installed.
!insertmacro MUI_PAGE_INSTFILES

# Finish page
!define MUI_FINISHPAGE_RUN 
!define MUI_FINISHPAGE_TEXT "$(FinishPageMessage)"
!define MUI_FINISHPAGE_RUN_TEXT "$(FinishPageRun)"
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchProduct"
!insertmacro MUI_PAGE_FINISH

# The uninstaller.
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES

#--------------------------------
# List of languages the installers are translated to

!include "TranslatedLanguages.nsh"

LicenseData "$(LyXLicenseData)"

#--------------------------------
# Reserve Files

# These files are inserted before other files in the data block
!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

#--------------------------------
# Installer sections

Section "!${PRODUCT_NAME}" SecCore
  SectionIn RO
SectionEnd
Section "$(SecFileAssocTitle)" SecFileAssoc
  StrCpy $CreateFileAssociations "true"
SectionEnd
Section "$(SecDesktopTitle)" SecDesktop
  StrCpy $CreateDesktopIcon "true"
SectionEnd

# Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(SecCoreDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# the installation section
!include "InstallActions-update.nsh"

#--------------------------------
# This hook function is called internally by NSIS on installer startup
Function .onInit

  # Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}
  
  # set the installer language to the Windows locale language
  System::Call "kernel32::GetUserDefaultLangID()i.a"
  
  # Check that the installer is not currently running
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
  
  # printer settings, needed to install the Metafile2eps printer
  ${if} ${AtLeastWinVista}
    StrCpy $PrinterConf "printui.exe"
  ${else}
    StrCpy $PrinterConf "rundll32.exe printui.dll,PrintUIEntry"
  ${endif}
  
  # check where LyX is installed
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
  ${endif} # end if $0 == 0
  ${if} $0 != "" # if it is found
   StrCpy $INSTDIR $0
   StrCpy $INSTDIR $INSTDIR -12 # delete the string "\bin\lyx.exe" or "\bin\lyx.bat"
  ${endif}
  
  # abort if the user doesn't have administrator privileges but LyX was installed as admin
  StrCpy $Answer ""
  StrCpy $UserName ""
  !insertmacro IsUserAdmin $Answer $UserName # macro from LyXUtils.nsh
  ${if} $Answer != "yes"
  ${andif} $ProductRootKey == "HKLM"
    MessageBox MB_OK|MB_ICONSTOP "$(NotAdmin)"
    Abort
  ${endif}
  
  # This can be reset to "true" in section SecDesktop.
  StrCpy $CreateDesktopIcon "false"
  
  ClearErrors
FunctionEnd

#--------------------------------

Function LaunchProduct
  Exec ${PRODUCT_BAT}
FunctionEnd

#--------------------------------
# The Uninstaller

 !include "Uninstall.nsh"
  

# eof
