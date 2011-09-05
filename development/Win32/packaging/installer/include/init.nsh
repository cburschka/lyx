/*

init.nsh

Initialization function

*/

#--------------------------------
# Installer initialization

!macro PRINTER_INIT

  ${If} ${AtLeastWinVista}
    StrCpy $PrinterConf "printui.exe"
  ${Else}
    StrCpy $PrinterConf "rundll32.exe printui.dll,PrintUIEntry"
  ${EndIf}

!macroend

Function .onInit

  ${IfNot} ${IsNT}
  ${OrIfNot} ${AtLeastWinXP}
    MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} ${APP_VERSION} requires Windows XP or later."
    Quit
  ${EndIf}
  
  # check that the installer is not currently running
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${BundleExeFile}.Instance") i .r1 ?e'
  Pop $R0
  ${if} $R0 != "0"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)"
   Abort
  ${endif}
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${ExeFile}.Instance") i .r1 ?e'
  Pop $R0
  ${if} $R0 != "0"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)"
   Abort
  ${endif}

  # check if LyX is already installed
  ReadRegStr $0 HKLM "${APP_UNINST_KEY}" "Publisher"
  ${if} $0 != ""
   MessageBox MB_OK|MB_ICONSTOP "$(StillInstalled)"
   Abort
  ${endif}

  !insertmacro PRINTER_INIT
  !insertmacro MULTIUSER_INIT
  
  # this can be reset to "true" in section SecDesktop
  StrCpy $CreateDesktopIcon "false"
  StrCpy $CreateFileAssociations "false"
 
  ${IfNot} ${Silent}
  
    !ifndef BUNDLE_IMAGEMAGICK & BUNDLE_GHOSTSCRIPT
    # Warn the user when no Administrator or Power user privileges are available
    # These privileges are required to install ImageMagick or Ghostscript
    ${If} $MultiUser.Privileges != "Admin"
    ${AndIf} $MultiUser.Privileges != "Power"
      MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_NO_PRIVILEDGES)
    ${EndIf}
    !endif
    
    # Show banner while installer is intializating 
    Banner::show /NOUNLOAD "Checking system"
  ${EndIf}
 
  Call SearchExternal
  #Call InitExternal
  
  !if ${SETUPTYPE} == BUNDLE
   # don't let the installer sections appear when the programs are already installed
   ${if} $PathBibTeXEditor != ""
    SectionSetText 3 "" # hides the corresponding uninstaller section, ${SecInstJabRef}
   ${endif}
  !endif
  
  ${IfNot} ${Silent}
    Banner::destroy
  ${EndIf}

FunctionEnd

# this function is called at first after starting the uninstaller
Function un.onInit

  !insertmacro PRINTER_INIT
  !insertmacro MULTIUSER_UNINIT

  # Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}

  # set registry root key
  ${if} $MultiUser.Privileges == "Admin"
  ${orif} $MultiUser.Privileges == "Power"
    SetShellVarContext all
  ${else}
   SetShellVarContext current
  ${endif}

  # Ascertain whether the user has sufficient privileges to uninstall.
  # abort when LyX was installed with admin permissions but the user doesn't have administrator privileges
  ReadRegStr $0 HKLM "${APP_UNINST_KEY}" "DisplayVersion"
  ${if} $0 != ""
  ${andif} $MultiUser.Privileges != "Admin"
  ${andif} $MultiUser.Privileges != "Power"
   MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)"
   Abort
  ${endif}
  # abort when LyX couldn't be found in the registry
  ${if} $0 == "" # check in HKCU
   ReadRegStr $0 HKCU "${APP_UNINST_KEY}" "DisplayVersion"
   ${if} $0 == ""
     MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)"
   ${endif}
  ${endif}
  
  # Macro to investigate name of LyX's preferences folders to be able remove them
  !insertmacro UnAppPreSuff $AppPre $AppSuff # macro from LyXUtils.nsh

  # test if MiKTeX was installed together with LyX
  ReadRegStr $0 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${if} $0 == "Yes${APP_SERIES_KEY}"
   SectionSetText 2 "MiKTeX" # names the corersponding uninstaller section
   StrCpy $LaTeXInstalled "MiKTeX"
   DeleteRegValue HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${else}
   SectionSetText 2 "" # hides the corresponding uninstaller section
  ${endif}
  
  # test if JabRef was installed together with LyX
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  ${if} $0 == "Yes${APP_SERIES_KEY}"
   SectionSetText 3 "JabRef" # names the corersponding uninstaller section
   StrCpy $JabRefInstalled "Yes"
   DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  ${else}
   SectionSetText 3 "" # hides the corresponding uninstaller section
  ${endif}

  # question message if the user really wants to uninstall LyX
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" IDYES +2 # continue if yes
  Abort

FunctionEnd

#--------------------------------
# User initialization

Var ComponentPath
Var LyXLangName

# COMPONENT can be LaTeX, ImageMagick and Ghostscript
!macro EXTERNAL_INIT COMPONENT

  # APP_REGKEY_SETUP = "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
  # where ${APP_NAME}${APP_SERIES_KEY} is something like LyX16
  ReadRegStr $ComponentPath SHELL_CONTEXT "${APP_REGKEY_SETUP}" "${COMPONENT} Path"
  
  # BIN_LATEX etc are defined in settings.nsh
  ${If} ${FileExists} "$ComponentPath\${BIN_${COMPONENT}}"
    # set variables like PathLaTeX
    StrCpy $Path${COMPONENT} $ComponentPath
  ${EndIf}

!macroend

Function InitUser

  # Get directories of components from registry
  
  !insertmacro EXTERNAL_INIT LaTeX
  
  # Get LyX language
  
  ReadRegStr $LyXLangName SHELL_CONTEXT "${APP_REGKEY_SETUP}" "LyX Language"
  
  ${If} $LyXLangName != ""
    StrCpy $LangName $LyXLangName
  ${EndIf}
  
FunctionEnd

#--------------------------------
# visible installer sections

Section "!${APP_NAME}" SecCore
  SectionIn RO
SectionEnd
Section "$(SecFileAssocTitle)" SecFileAssoc
  StrCpy $CreateFileAssociations "true"
SectionEnd
Section "$(SecDesktopTitle)" SecDesktop
  StrCpy $CreateDesktopIcon "true"
SectionEnd

!if ${SETUPTYPE} == BUNDLE
 Section /o "$(SecInstJabRefTitle)" SecInstJabRef
  AddSize 5000
  StrCpy $InstallJabRef "true"
 SectionEnd
!endif

# Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(SecCoreDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "$(SecFileAssocDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!if ${SETUPTYPE} == BUNDLE
 !insertmacro MUI_DESCRIPTION_TEXT ${SecInstJabRef} "$(SecInstJabRefDescription)"
!endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END

