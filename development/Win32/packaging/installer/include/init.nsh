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

Section /o "Afrikaans" SecDAfrikaans
 StrCpy $DictCodes "af_ZA$DictCodes"
 AddSize 1440
SectionEnd

Section /o "Arabic" SecDArabic
 StrCpy $DictCodes "ar_DZ$DictCodes"
 AddSize 2500
SectionEnd

Section /o "Armenian" SecDArmenian
 StrCpy $DictCodes "hy_AM$DictCodes"
 AddSize 2000
SectionEnd

Section /o "Bahasa Indonesia" SecDIndonesian
 StrCpy $DictCodes "id_ID$DictCodes"
 AddSize 217
SectionEnd

Section /o "Bahasa Melayu" SecDMalayan
 StrCpy $DictCodes "ms_MY$DictCodes"
 AddSize 227
SectionEnd

Section /o "Belarusian" SecDBelarusian 
 StrCpy $DictCodes "be_BY$DictCodes"
 AddSize 1040
SectionEnd

Section /o "Brezhoneg" SecDBreton 
 StrCpy $DictCodes "br_FR$DictCodes"
 AddSize 11000
SectionEnd

Section /o "Bulgarian" SecDBulgarian
 StrCpy $DictCodes "bg_BG$DictCodes"
 AddSize 985
SectionEnd

Section /o "Català" SecDCatalanian
 StrCpy $DictCodes "ca_ES$DictCodes"
 AddSize 1210
SectionEnd

Section /o "Ceština" SecDCzech
 StrCpy $DictCodes "cs_CZ$DictCodes"
 AddSize 2190
SectionEnd

Section /o "Cymraeg" SecDWelsh 
 StrCpy $DictCodes "cy_GB$DictCodes"
 AddSize 1540
SectionEnd

Section /o "Dansk" SecDDanish
 StrCpy $DictCodes "da_DK$DictCodes"
 AddSize 2470
SectionEnd

Section /o "German (A)" SecDGermanAT
 StrCpy $DictCodes "de_AT$DictCodes"
 AddSize 1000
SectionEnd

Section /o "German (CH)" SecDGermanCH
 StrCpy $DictCodes "de_CH$DictCodes"
 AddSize 1000
SectionEnd

Section "German (D)" SecDGermanD
 # already installed by default
 #StrCpy $DictCodes "de_DE$DictCodes"
 AddSize 2650
SectionEnd

Section /o "Greek" SecDGreek
 StrCpy $DictCodes "el_GR$DictCodes"
 AddSize 6550
SectionEnd

Section /o "Eesti" SecDEstonian
 StrCpy $DictCodes "et_EE$DictCodes"
 AddSize 4400
SectionEnd

# enable this for LyX 2.1!
#Section /o "English (AU)" SecDEnglishAU
# StrCpy $DictCodes "en_AU$DictCodes"
# AddSize 587
#SectionEnd

Section /o "English (CA)" SecDEnglishCA
 StrCpy $DictCodes "en_CA$DictCodes"
 AddSize 690
SectionEnd

Section "English (GB)" SecDEnglishGB
 # already installed by default
 #StrCpy $DictCodes "en_GB$DictCodes"
 AddSize 757
SectionEnd

# enable this for LyX 2.1!
#Section /o "English (NZ)" SecDEnglishNZ
# StrCpy $DictCodes "en_NZ$DictCodes"
# AddSize 551
#SectionEnd

Section "English (US)" SecDEnglishUS
 # already installed by default
 #StrCpy $DictCodes "en_US$DictCodes"
 AddSize 688
SectionEnd

Section "Español (ES)" SecDSpanishES
 # already installed by default
 #StrCpy $DictCodes "es_ES$DictCodes"
 AddSize 974
SectionEnd

Section "Español (MX)" SecDSpanishMX
 # already installed by default
 #StrCpy $DictCodes "es_MX$DictCodes"
 AddSize 924
SectionEnd

Section /o "Esperanto" SecDEsperanto
 StrCpy $DictCodes "eo_EO$DictCodes"
 AddSize 389
SectionEnd

Section /o "Euskara" SecDBasque
 StrCpy $DictCodes "eu_ES$DictCodes"
 AddSize 4850
SectionEnd

Section /o "Farsi" SecDFarsi
 StrCpy $DictCodes "fa_IR$DictCodes"
 AddSize 6710
SectionEnd

Section "Français" SecDFrench
 # already installed by default
 #StrCpy $DictCodes "fr_FR$DictCodes"
 AddSize 1200
SectionEnd

Section /o "Gaeilge" SecDGaelic
 StrCpy $DictCodes "ga_IR$DictCodes"
 AddSize 1090
SectionEnd

Section /o "Gàidhlig" SecDScottish
 StrCpy $DictCodes "gd_GB$DictCodes"
 AddSize 2460
SectionEnd

Section /o "Galego" SecDGalician
 StrCpy $DictCodes "gl_ES$DictCodes"
 AddSize 916
SectionEnd

Section /o "Hebrew" SecDHebrew
 StrCpy $DictCodes "he_IL$DictCodes"
 AddSize 3120
SectionEnd

Section /o "Hrvatski" SecDCroatian
 StrCpy $DictCodes "hr_HR$DictCodes"
 AddSize 2240
SectionEnd

Section /o "Magyar" SecDHungarian
 StrCpy $DictCodes "hu_HU$DictCodes"
 AddSize 3380
SectionEnd

Section /o "Interlingua" SecDInterlingua
 StrCpy $DictCodes "ia_IA$DictCodes"
 AddSize 649
SectionEnd

Section /o "Íslenska" SecDIcelandic
 StrCpy $DictCodes "is_IS$DictCodes"
 AddSize 2320
SectionEnd

Section /o "Italiano" SecDItalian
 StrCpy $DictCodes "it_IT$DictCodes"
 AddSize 1300
SectionEnd

Section /o "Kazakh" SecDKazakh
 StrCpy $DictCodes "kk_KZ$DictCodes"
 AddSize 2120
SectionEnd

Section /o "Korean" SecDKorean
 StrCpy $DictCodes "ko_KR$DictCodes"
 AddSize 15200
SectionEnd

Section /o "Latina" SecDLatin
 StrCpy $DictCodes "la_LA$DictCodes"
 AddSize 1250
SectionEnd

Section /o "Lietuviu" SecDLithuanian
 StrCpy $DictCodes "lt_LT$DictCodes"
 AddSize 1320
SectionEnd

Section /o "Latviešu" SecDLatvian
 StrCpy $DictCodes "lv_LV$DictCodes"
 AddSize 2140
SectionEnd

Section /o "Nederlands" SecDDutch
 StrCpy $DictCodes "nl_NL$DictCodes"
 AddSize 1820
SectionEnd

Section /o "Norsk (Bokmål)" SecDNorwegianNB
 StrCpy $DictCodes "nb_NO$DictCodes"
 AddSize 4890
SectionEnd

Section /o "Norsk (Nynorsk)" SecDNorwegianNN
 StrCpy $DictCodes "nn_NO$DictCodes"
 AddSize 2890
SectionEnd

Section /o "Polski" SecDPolish
 StrCpy $DictCodes "pl_PL$DictCodes"
 AddSize 4540
SectionEnd

Section /o "Português (BR)" SecDPortugueseBR
 StrCpy $DictCodes "pt_BR$DictCodes"
 AddSize 5280
SectionEnd

Section /o "Português (PT)" SecDPortuguesePT
 StrCpy $DictCodes "pt_PT$DictCodes"
 AddSize 1460
SectionEnd

Section /o "Româna" SecDRomanian
 StrCpy $DictCodes "ro_RO$DictCodes"
 AddSize 1930
SectionEnd

Section "Russian" SecDRussian
 # already installed by default
 #StrCpy $DictCodes "ru_RU$DictCodes"
 AddSize 1920
SectionEnd

Section /o "Serbšcina (Dolno)" SecDSorbianD
 StrCpy $DictCodes "db_DE$DictCodes"
 AddSize 904
SectionEnd

Section /o "Serbšcina (Horno)" SecDSorbianH
 StrCpy $DictCodes "hb_DE$DictCodes"
 AddSize 740
SectionEnd

Section /o "Shqipe" SecDAlbanian
 StrCpy $DictCodes "sq_AL$DictCodes"
 AddSize 2400
SectionEnd

Section /o "Slovenšcina" SecDSlowenian
 StrCpy $DictCodes "sl_SI$DictCodes"
 AddSize 2840
SectionEnd

Section /o "Slovenský" SecDSlowakian
 StrCpy $DictCodes "sk_SK$ThesCodes"
 AddSize 4090
SectionEnd

Section /o "Srpski" SecDSerbian
 StrCpy $DictCodes "sr_RS$DictCodes"
 AddSize 3460
SectionEnd

Section /o "Svenska" SecDSwedish
 StrCpy $DictCodes "sv_SE$DictCodes"
 AddSize 1030
SectionEnd
 
Section /o "Thai" SecDThai
 StrCpy $DictCodes "th_TH$DictCodes"
 AddSize 351
SectionEnd

Section /o "Ukrainian" SecDUkrainian
 StrCpy $DictCodes "uk_UA$DictCodes"
 AddSize 2620
SectionEnd

Section /o "Vietnamese" SecDVietnamese
 StrCpy $DictCodes "vi_VN$DictCodes"
 AddSize 39600
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

