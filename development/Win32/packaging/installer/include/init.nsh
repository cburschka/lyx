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

SectionGroup "Dictionaries" SecDictionaries

Section /o "German (A)" SecDGermanAT
 StrCpy $DictCodes "de_AT$DictCodes"
 AddSize 1000
SectionEnd

Section /o "German (CH)" SecDGermanCH
 StrCpy $DictCodes "de_CH$DictCodes"
 AddSize 1000
SectionEnd

Section "German (D)" SecDGermanD
 StrCpy $DictCodes "de_DE$DictCodes"
 AddSize 2650
SectionEnd

Section "Magyar" SecDHungarian
 StrCpy $DictCodes "hu_HU$DictCodes"
 AddSize 3380
SectionEnd

Section "Serbšcina" SecDSorbian
 StrCpy $DictCodes "db_DE$DictCodes"
 AddSize 904
SectionEnd

SectionGroupEnd

SectionGroup "Thesaurus" SecThesaurus

Section /o "Bulgarian" SecTBulgarian
 StrCpy $ThesCodes "bg_BG$ThesCodes"
 AddSize 3020
SectionEnd

Section /o "Català" SecTCatalan
 StrCpy $ThesCodes "ca_ES$ThesCodes"
 AddSize 731
SectionEnd

Section /o "Ceština" SecTCzech
 StrCpy $ThesCodes "cs_CZ$ThesCodes"
 AddSize 635
SectionEnd

Section /o "Dansk" SecTDanish
 StrCpy $ThesCodes "da_DK$ThesCodes"
 AddSize 2360
SectionEnd

Section /o "Deutsch (D/A)" SecTGermanDA
 StrCpy $ThesCodes "de_DE$ThesCodes"
 AddSize 5360
SectionEnd

Section /o "Deutsch (CH)" SecTGermanCH
 StrCpy $ThesCodes "de_CH$ThesCodes"
 AddSize 5360
SectionEnd

Section /o "English (GB)" SecTEnglishGB
 StrCpy $ThesCodes "en_GB$ThesCodes"
 AddSize 20600
SectionEnd

Section /o "English (US/AU)" SecTEnglishUSAU
 StrCpy $ThesCodes "en_US$ThesCodes"
 AddSize 20600
SectionEnd

Section /o "Español" SecTSpanish
 StrCpy $ThesCodes "es_ES$ThesCodes"
 AddSize 2860
SectionEnd

Section /o "Français" SecTFrench
 StrCpy $ThesCodes "fr_FR$ThesCodes"
 AddSize 5060
SectionEnd

Section /o "Gaeilge" SecTGaelic
 StrCpy $ThesCodes "ga_IR$ThesCodes"
 AddSize 30600
SectionEnd

Section /o "Greek" SecTGreek
 StrCpy $ThesCodes "el_GR$ThesCodes"
 AddSize 903
SectionEnd

Section /o "Italiano" SecTItalian
 StrCpy $ThesCodes "it_IT$ThesCodes"
 AddSize 2640
SectionEnd

Section /o "Magyar" SecTHungarian
 StrCpy $ThesCodes "hu_HU$ThesCodes"
 AddSize 632
SectionEnd

Section /o "Norsk" SecTNorwegian
 StrCpy $ThesCodes "no_NO$ThesCodes"
 AddSize 2470
SectionEnd

Section /o "Polski" SecTPolish
 StrCpy $ThesCodes "pl_PL$ThesCodes"
 AddSize 5580
SectionEnd

Section /o "Português" SecTPortuguese
 StrCpy $ThesCodes "pt_PT$ThesCodes"
 AddSize 855
SectionEnd

Section /o "Româna" SecTRomanian
 StrCpy $ThesCodes "ro_RO$ThesCodes"
 AddSize 3640
SectionEnd

Section /o "Russian" SecTRussian
 StrCpy $ThesCodes "ru_RU$ThesCodes"
 AddSize 2080
SectionEnd

Section /o "Slovenšcina" SecTSlowenian
 StrCpy $ThesCodes "sl_SI$ThesCodes"
 AddSize 107
SectionEnd

Section /o "Slovenský" SecTSlowakian
 StrCpy $ThesCodes "sk_SK$ThesCodes"
 AddSize 907
SectionEnd

Section /o "Svenska" SecTSwedish
 StrCpy $ThesCodes "sv_SE$ThesCodes"
 AddSize 720
SectionEnd

SectionGroupEnd

# Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(SecCoreDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "$(SecFileAssocDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(SecDesktopDescription)"
!if ${SETUPTYPE} == BUNDLE
 !insertmacro MUI_DESCRIPTION_TEXT ${SecInstJabRef} "$(SecInstJabRefDescription)"
!endif
!insertmacro MUI_FUNCTION_DESCRIPTION_END

