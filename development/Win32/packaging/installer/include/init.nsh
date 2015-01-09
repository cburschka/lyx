/*
init.nsh

Initialization functions
*/

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
  AddSize ${SIZE_JABREF}
  StrCpy $InstallJabRef "true"
 SectionEnd
!endif

# Expand the list of dictionaries by default as this was requested by several
# users. For the thesaurus this is was not requested because this section
# is by default empty.
SectionGroup /e "Dictionaries" SecDictionaries

Section /o "Afrikaans" SecDAfrikaans
 StrCpy $DictCodes "af_ZA,$DictCodes"
 AddSize 1440
SectionEnd

Section /o "Arabic" SecDArabic
 StrCpy $DictCodes "ar_DZ,$DictCodes"
 AddSize 2500
SectionEnd

Section /o "Armenian" SecDArmenian
 StrCpy $DictCodes "hy_AM,$DictCodes"
 AddSize 2000
SectionEnd

Section /o "Bahasa Indonesia" SecDIndonesian
 StrCpy $DictCodes "id_ID,$DictCodes"
 AddSize 217
SectionEnd

Section /o "Bahasa Melayu" SecDMalayan
 StrCpy $DictCodes "ms_MY,$DictCodes"
 AddSize 227
SectionEnd

Section /o "Belarusian" SecDBelarusian 
 StrCpy $DictCodes "be_BY,$DictCodes"
 AddSize 1730
SectionEnd

Section /o "Brezhoneg" SecDBreton 
 StrCpy $DictCodes "br_FR,$DictCodes"
 AddSize 5510
SectionEnd

Section /o "Bulgarian" SecDBulgarian
 StrCpy $DictCodes "bg_BG,$DictCodes"
 AddSize 985
SectionEnd

Section /o "Català" SecDCatalanian
 StrCpy $DictCodes "ca_ES,$DictCodes"
 AddSize 1210
SectionEnd

Section /o "Ceština" SecDCzech
 StrCpy $DictCodes "cs_CZ,$DictCodes"
 AddSize 2190
SectionEnd

Section /o "Coptic" SecDCoptic
 StrCpy $DictCodes "cop_EG,$DictCodes"
 AddSize 151
SectionEnd

Section /o "Cymraeg" SecDWelsh 
 StrCpy $DictCodes "cy_GB,$DictCodes"
 AddSize 1540
SectionEnd

Section /o "Dansk" SecDDanish
 StrCpy $DictCodes "da_DK,$DictCodes"
 AddSize 2470
SectionEnd

Section /o "German (A)" SecDGermanAT
 StrCpy $DictCodes "de_AT,$DictCodes"
 AddSize 3620
SectionEnd

Section /o "German (CH)" SecDGermanCH
 StrCpy $DictCodes "de_CH,$DictCodes"
 AddSize 3620
SectionEnd

Section "German (D)" SecDGermanD
 # already installed by default
 SectionIn RO
 #StrCpy $DictCodes "de_DE,$DictCodes"
 AddSize 3620
SectionEnd

Section /o "Greek" SecDGreek
 StrCpy $DictCodes "el_GR,$DictCodes"
 AddSize 6550
SectionEnd

Section /o "Eesti" SecDEstonian
 StrCpy $DictCodes "et_EE,$DictCodes"
 AddSize 4400
SectionEnd

Section /o "English (AU)" SecDEnglishAU
 StrCpy $DictCodes "en_AU,$DictCodes"
 AddSize 587
SectionEnd

Section /o "English (CA)" SecDEnglishCA
 StrCpy $DictCodes "en_CA,$DictCodes"
 AddSize 531
SectionEnd

Section "English (GB)" SecDEnglishGB
 # already installed by default
 SectionIn RO
 #StrCpy $DictCodes "en_GB,$DictCodes"
 AddSize 652
SectionEnd

Section /o "English (NZ)" SecDEnglishNZ
 StrCpy $DictCodes "en_NZ,$DictCodes"
 AddSize 551
SectionEnd

Section "English (US)" SecDEnglishUS
 # already installed by default
 SectionIn RO
 #StrCpy $DictCodes "en_US,$DictCodes"
 AddSize 530
SectionEnd

Section "Español (ES)" SecDSpanishES
 # already installed by default
 SectionIn RO
 #StrCpy $DictCodes "es_ES,$DictCodes"
 AddSize 974
SectionEnd

Section "Español (MX)" SecDSpanishMX
 # already installed by default
 SectionIn RO
 #StrCpy $DictCodes "es_MX,$DictCodes"
 AddSize 924
SectionEnd

Section /o "Esperanto" SecDEsperanto
 StrCpy $DictCodes "eo_EO,$DictCodes"
 AddSize 389
SectionEnd

Section /o "Euskara" SecDBasque
 StrCpy $DictCodes "eu_ES,$DictCodes"
 AddSize 4850
SectionEnd

Section /o "Farsi" SecDFarsi
 StrCpy $DictCodes "fa_IR,$DictCodes"
 AddSize 6710
SectionEnd

Section "Français" SecDFrench
 # already installed by default
 SectionIn RO
 #StrCpy $DictCodes "fr_FR,$DictCodes"
 AddSize 1200
SectionEnd

Section /o "Français (Canada)" SecDFrenchCanada
 StrCpy $DictCodes "fr_CA,$DictCodes"
 AddSize 1390
SectionEnd

Section /o "Gaeilge" SecDGaelic
 StrCpy $DictCodes "ga_IR,$DictCodes"
 AddSize 1090
SectionEnd

Section /o "Gàidhlig" SecDScottish
 StrCpy $DictCodes "gd_GB,$DictCodes"
 AddSize 3090
SectionEnd

Section /o "Galego" SecDGalician
 StrCpy $DictCodes "gl_ES,$DictCodes"
 AddSize 3911
SectionEnd

Section /o "Hebrew" SecDHebrew
 StrCpy $DictCodes "he_IL,$DictCodes"
 AddSize 3120
SectionEnd

Section /o "Hrvatski" SecDCroatian
 StrCpy $DictCodes "hr_HR,$DictCodes"
 AddSize 2240
SectionEnd

Section /o "Magyar" SecDHungarian
 StrCpy $DictCodes "hu_HU,$DictCodes"
 AddSize 3380
SectionEnd

Section /o "Hindi" SecDHindi
 StrCpy $DictCodes "hi_IN,$DictCodes"
 AddSize 1900
SectionEnd

Section /o "Interlingua" SecDInterlingua
 StrCpy $DictCodes "ia_IA,$DictCodes"
 AddSize 613
SectionEnd

Section /o "Íslenska" SecDIcelandic
 StrCpy $DictCodes "is_IS,$DictCodes"
 AddSize 2320
SectionEnd

Section /o "Italiano" SecDItalian
 StrCpy $DictCodes "it_IT,$DictCodes"
 AddSize 1380
SectionEnd

Section /o "Kazakh" SecDKazakh
 StrCpy $DictCodes "kk_KZ,$DictCodes"
 AddSize 2120
SectionEnd

Section /o "Korean" SecDKorean
 StrCpy $DictCodes "ko_KR,$DictCodes"
 AddSize 16540
SectionEnd

Section /o "Latina" SecDLatin
 StrCpy $DictCodes "la_LA,$DictCodes"
 AddSize 2040
SectionEnd

Section /o "Lietuviu" SecDLithuanian
 StrCpy $DictCodes "lt_LT,$DictCodes"
 AddSize 1320
SectionEnd

Section /o "Latviešu" SecDLatvian
 StrCpy $DictCodes "lv_LV,$DictCodes"
 AddSize 2243
SectionEnd

Section /o "Marathi" SecDMarathi
 StrCpy $DictCodes "mr_IN,$DictCodes"
 AddSize 5290
SectionEnd

Section /o "Nederlands" SecDDutch
 StrCpy $DictCodes "nl_NL,$DictCodes"
 AddSize 1820
SectionEnd

Section /o "Norsk (Bokmål)" SecDNorwegianNB
 StrCpy $DictCodes "nb_NO,$DictCodes"
 AddSize 3992
SectionEnd

Section /o "Norsk (Nynorsk)" SecDNorwegianNN
 StrCpy $DictCodes "nn_NO,$DictCodes"
 AddSize 1540
SectionEnd

Section /o "Occitan" SecDOccitan
 StrCpy $DictCodes "oc_FR,$DictCodes"
 AddSize 31710
SectionEnd

Section /o "Polski" SecDPolish
 StrCpy $DictCodes "pl_PL,$DictCodes"
 AddSize 4540
SectionEnd

Section /o "Português (BR)" SecDPortugueseBR
 StrCpy $DictCodes "pt_BR,$DictCodes"
 AddSize 5280
SectionEnd

Section /o "Português (PT)" SecDPortuguesePT
 StrCpy $DictCodes "pt_PT,$DictCodes"
 AddSize 1490
SectionEnd

Section /o "Româna" SecDRomanian
 StrCpy $DictCodes "ro_RO,$DictCodes"
 AddSize 2255
SectionEnd

Section /o "Russian" SecDRussian
 StrCpy $DictCodes "ru_RU,$DictCodes"
 AddSize 1920
SectionEnd

Section /o "Serbšcina (Dolno)" SecDSorbianD
 StrCpy $DictCodes "dsb_DE,$DictCodes"
 AddSize 1035
SectionEnd

Section /o "Serbšcina (Horno)" SecDSorbianH
 StrCpy $DictCodes "hsb_DE,$DictCodes"
 AddSize 740
SectionEnd

Section /o "Shqipe" SecDAlbanian
 StrCpy $DictCodes "sq_AL,$DictCodes"
 AddSize 2400
SectionEnd

Section /o "Slovenšcina" SecDSlovenian
 StrCpy $DictCodes "sl_SI,$DictCodes"
 AddSize 2910
SectionEnd

Section /o "Slovenský" SecDSlovakian
 StrCpy $DictCodes "sk_SK,$DictCodes"
 AddSize 3310
SectionEnd

Section /o "Srpski (Cirilica)" SecDSerbianC
 StrCpy $DictCodes "sr_RS,$DictCodes"
 AddSize 4401
SectionEnd

Section /o "Srpski (Latinica)" SecDSerbianL
 StrCpy $DictCodes "sr_RS-Latin,$DictCodes"
 AddSize 2843
SectionEnd

Section /o "Svenska" SecDSwedish
 StrCpy $DictCodes "sv_SE,$DictCodes"
 AddSize 2028
SectionEnd

Section /o "Tamil" SecDTamil
 StrCpy $DictCodes "ta_IN,$DictCodes"
 AddSize 5911
SectionEnd

Section /o "Telugu" SecDTelugu
 StrCpy $DictCodes "te_IN,$DictCodes"
 AddSize 3400
SectionEnd
 
Section /o "Thai" SecDThai
 StrCpy $DictCodes "th_TH,$DictCodes"
 AddSize 351
SectionEnd

Section /o "Türkmençe" SecDTurkmen
 StrCpy $DictCodes "tk_TM,$DictCodes"
 AddSize 950
SectionEnd

Section /o "Türkçe" SecDTurkish
 StrCpy $DictCodes "tr_TR,$DictCodes"
 AddSize 8870
SectionEnd

Section /o "Ukrainian" SecDUkrainian
 StrCpy $DictCodes "uk_UA,$DictCodes"
 AddSize 3077
SectionEnd

Section /o "Urdu" SecDUrdu
 StrCpy $DictCodes "ur_PK,$DictCodes"
 AddSize 1401
SectionEnd

Section /o "Vietnamese" SecDVietnamese
 StrCpy $DictCodes "vi_VN,$DictCodes"
 AddSize 40
SectionEnd

SectionGroupEnd


SectionGroup "Thesaurus" SecThesaurus

Section /o "Bulgarian" SecTBulgarian
 StrCpy $ThesCodes "bg_BG,$ThesCodes"
 AddSize 3020
SectionEnd

Section /o "Català" SecTCatalan
 StrCpy $ThesCodes "ca_ES,$ThesCodes"
 AddSize 731
SectionEnd

Section /o "Ceština" SecTCzech
 StrCpy $ThesCodes "cs_CZ,$ThesCodes"
 AddSize 635
SectionEnd

Section /o "Dansk" SecTDanish
 StrCpy $ThesCodes "da_DK,$ThesCodes"
 AddSize 2360
SectionEnd

Section /o "Deutsch (D/A)" SecTGermanDA
 StrCpy $ThesCodes "de_DE,$ThesCodes"
 AddSize 14600
SectionEnd

Section /o "Deutsch (CH)" SecTGermanCH
 StrCpy $ThesCodes "de_CH,$ThesCodes"
 AddSize 14600
SectionEnd

Section /o "English (GB)" SecTEnglishGB
 StrCpy $ThesCodes "en_GB,$ThesCodes"
 AddSize 14300
SectionEnd

Section /o "English (US/AU)" SecTEnglishUSAU
 StrCpy $ThesCodes "en_US,$ThesCodes"
 AddSize 21600
SectionEnd

Section /o "Español" SecTSpanish
 StrCpy $ThesCodes "es_ES,$ThesCodes"
 AddSize 2860
SectionEnd

Section /o "Français" SecTFrench
 StrCpy $ThesCodes "fr_FR,$ThesCodes"
 AddSize 5060
SectionEnd

Section /o "Gaeilge" SecTGaelic
 StrCpy $ThesCodes "ga_IR,$ThesCodes"
 AddSize 30600
SectionEnd

Section /o "Galego" SecTGalician
 StrCpy $ThesCodes "gl_ES,$ThesCodes"
 AddSize 510
SectionEnd

Section /o "Greek" SecTGreek
 StrCpy $ThesCodes "el_GR,$ThesCodes"
 AddSize 903
SectionEnd

Section /o "Íslenska" SecTIcelandic
 StrCpy $ThesCodes "is_IS,$ThesCodes"
 AddSize 63
SectionEnd

Section /o "Italiano" SecTItalian
 StrCpy $ThesCodes "it_IT,$ThesCodes"
 AddSize 2520
SectionEnd

Section /o "Magyar" SecTHungarian
 StrCpy $ThesCodes "hu_HU,$ThesCodes"
 AddSize 632
SectionEnd

Section /o "Norsk (Bokmål)" SecTNorwegianNB
 StrCpy $ThesCodes "nb_NO,$ThesCodes"
 AddSize 2535
SectionEnd

Section /o "Polski" SecTPolish
 StrCpy $ThesCodes "pl_PL,$ThesCodes"
 AddSize 5580
SectionEnd

Section /o "Português" SecTPortuguese
 StrCpy $ThesCodes "pt_PT,$ThesCodes"
 AddSize 860
SectionEnd

Section /o "Româna" SecTRomanian
 StrCpy $ThesCodes "ro_RO,$ThesCodes"
 AddSize 3650
SectionEnd

Section /o "Russian" SecTRussian
 StrCpy $ThesCodes "ru_RU,$ThesCodes"
 AddSize 2080
SectionEnd

Section /o "Slovenšcina" SecTSlovenian
 StrCpy $ThesCodes "sl_SI,$ThesCodes"
 AddSize 1110
SectionEnd

Section /o "Slovenský" SecTSlovakian
 StrCpy $ThesCodes "sk_SK,$ThesCodes"
 AddSize 930
SectionEnd

Section /o "Svenska" SecTSwedish
 StrCpy $ThesCodes "sv_SE,$ThesCodes"
 AddSize 720
SectionEnd

Section /o "Ukrainian" SecTUkrainian
 StrCpy $ThesCodes "uk_UA,$ThesCodes"
 AddSize 1309
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
!insertmacro MUI_DESCRIPTION_TEXT ${SecDictionaries} "$(SecDictionariesDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${SecThesaurus} "$(SecThesaurusDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


#--------------------------------
# Installer initialization

!macro PRINTER_INIT

  ${If} ${AtLeastWinVista}
    StrCpy $PrinterConf "printui.exe"
  ${Else}
    StrCpy $PrinterConf "rundll32.exe printui.dll,PrintUIEntry"
  ${EndIf}

!macroend

# .onInit must be here after the section definition because we have to set
# the selection states of the dictionary sections
Function .onInit

  ${IfNot} ${IsNT}
  ${OrIfNot} ${AtLeastWinXP}
    MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} ${APP_VERSION} requires Windows XP or later." /SD IDOK
    Quit
  ${EndIf}
  
  # check that the installer is not currently running
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${BundleExeFile}.Instance") i .r1 ?e'
  Pop $R0
  ${if} $R0 != "0"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)" /SD IDOK
   Abort
  ${endif}
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${ExeFile}.Instance") i .r1 ?e'
  Pop $R0
  ${if} $R0 != "0"
   MessageBox MB_OK|MB_ICONSTOP "$(InstallRunning)" /SD IDOK
   Abort
  ${endif}
  
  # FIXME: check that LyX is not currently running
  #System::Call 'kernel32::CreateMutexA(i 0, i 0, t "LyX.exe.Instance") i .r1 ?e'
  #Pop $R0
  #MessageBox MB_OK "$R0"
  #${if} $R0 != "0"
  # MessageBox MB_OK|MB_ICONSTOP "$(LyXRunning)" 
  # Abort
  #${endif}
  
  !insertmacro MULTIUSER_INIT
  
  # check if this LyX version is already installed
  ${if} $MultiUser.Privileges == "Admin"
  ${orif} $MultiUser.Privileges == "Power"
   ReadRegStr $0 HKLM "${APP_UNINST_KEY}" "DisplayIcon"
  ${else}
   ReadRegStr $0 HKCU "${APP_UNINST_KEY}" "DisplayIcon"
   # handle also the case that LyX is already installed in HKLM
   ${if} $0 == ""
    ReadRegStr $0 HKLM "${APP_UNINST_KEY}" "DisplayIcon"
   ${endif}
  ${endif}
  ${if} $0 != ""
   # check if the uninstaller was acidentally deleted
   # if so don't bother the user if he realy wants to install a new LyX over an existing one
   # because he won't have a chance to deny this
   StrCpy $4 $0 -10 # remove '\bin\lyx,0'
   # (for FileCheck the variables $0 and $1 cannot be used)
   !insertmacro FileCheck $5 "Uninstall-LyX.exe" "$4" # macro from LyXUtils.nsh
   ${if} $5 == "False"
    Goto ForceInstallation
   ${endif}
   # installing over an existing installation of the same LyX release is not necessary
   # if the users does this he most probably has a problem with LyX that can better be solved
   # by reinstalling LyX
   # for beta and other test releases over-installing can even cause errors
   MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION "$(AlreadyInstalled)" /SD IDNO IDYES ForceInstallation 
   Abort
   ForceInstallation:
  ${endif}
  
  # check if there is an existing LyX installation of the same LyX series
  # we usually don't release more than 10 versions so with 20 we are safe to check if a newer version is installed
  IntOp $4 ${APP_VERSION_REVISION} + 20
  ${for} $5 0 $4
   ${if} $MultiUser.Privileges == "Admin"
   ${orif} $MultiUser.Privileges == "Power"
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}${APP_VERSION_MAJOR}${APP_VERSION_MINOR}$5" "DisplayVersion"
    # also check for an emergency release
    ${if} $0 == ""
     ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}${APP_VERSION_MAJOR}${APP_VERSION_MINOR}$51" "DisplayVersion"
    ${endif}
   ${else}
    ReadRegStr $0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}${APP_VERSION_MAJOR}${APP_VERSION_MINOR}$5" "DisplayVersion"
    # also check for an emergency release
    ${if} $0 == ""
     ReadRegStr $0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}${APP_VERSION_MAJOR}${APP_VERSION_MINOR}$51" "DisplayVersion"
    ${endif}
   ${endif}
   ${if} $0 != ""
    StrCpy $R5 $0 # store the read version number
    StrCpy $OldVersionNumber "${APP_VERSION_MAJOR}${APP_VERSION_MINOR}$5"
    # we don't stop here because we want the latest installed version
   ${endif} 
  ${next}
  
  ${if} $OldVersionNumber > ${APP_SERIES_KEY}
   # store the version number and reformat it temporarily for the error message
   StrCpy $R0 $OldVersionNumber
   StrCpy $OldVersionNumber $R5
   MessageBox MB_OK|MB_ICONSTOP "$(NewerInstalled)" /SD IDOK
   StrCpy $OldVersionNumber $R0
   Abort
  ${endif}

  !insertmacro PRINTER_INIT
  
  # this can be reset to "true" in section SecDesktop
  StrCpy $CreateDesktopIcon "false"
  StrCpy $CreateFileAssociations "false"
 
  ${IfNot} ${Silent}
    # Show banner while installer is intializating 
    Banner::show /NOUNLOAD "Checking system"
  ${EndIf}
 
  Call SearchExternal
  
  !if ${SETUPTYPE} == BUNDLE
   # don't let the installer sections appear when the programs are already installed
   ${if} $PathBibTeXEditor != ""
    SectionSetText 3 "" # hides the corresponding uninstaller section, ${SecInstJabRef}
   ${endif}
  !endif
  
  # select sections of already installed spell-checker dictionaries, make them read-only
  # and set the necessary size to 0 bytes
  StrCpy $String $FoundDict
  StrCpy $Search "af_ZA"
  Call StrPoint # function from LyXUtils.nsh
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDAfrikaans} $0
   SectionSetSize ${SecDAfrikaans} 0
  ${endif}
  StrCpy $Search "ar_DZ"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDArabic} $0
   SectionSetSize ${SecDArabic} 0
  ${endif}
  StrCpy $Search "hy_AM"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDArmenian} $0
   SectionSetSize ${SecDArmenian} 0
  ${endif}
  StrCpy $Search "id_ID"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDIndonesian} $0
   SectionSetSize ${SecDIndonesian} 0
  ${endif}
  StrCpy $Search "ms_MY"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDMalayan} $0
   SectionSetSize ${SecDMalayan} 0
  ${endif}
  StrCpy $Search "be_BY"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDBelarusian} $0
   SectionSetSize ${SecDBelarusian} 0
  ${endif}
  StrCpy $Search "br_FR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDBreton} $0
   SectionSetSize ${SecDBreton} 0
  ${endif}
  StrCpy $Search "bg_BG"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDBulgarian} $0
   SectionSetSize ${SecDBulgarian} 0
  ${endif}
  StrCpy $Search "ca_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDCatalanian} $0
   SectionSetSize ${SecDCatalanian} 0
  ${endif}
  StrCpy $Search "cs_CZ"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDCzech} $0
   SectionSetSize ${SecDCzech} 0
  ${endif}
 
  StrCpy $Search "cop_EG"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDCoptic} $0
   SectionSetSize ${SecDCoptic} 0
  ${endif}
  StrCpy $Search "cy_GB"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDWelsh} $0
   SectionSetSize ${SecDWelsh} 0
  ${endif}
  StrCpy $Search "da_DK"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDDanish} $0
   SectionSetSize ${SecDDanish} 0
  ${endif}
  StrCpy $Search "de_AT"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDGermanAT} $0
   SectionSetSize ${SecDGermanAT} 0
  ${endif}
  StrCpy $Search "de_CH"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDGermanCH} $0
   SectionSetSize ${SecDGermanCH} 0
  ${endif}
  StrCpy $Search "de_DE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDGermanD} $0
   SectionSetSize ${SecDGermanD} 0
  ${endif}
  StrCpy $Search "el_GR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDGreek} $0
   SectionSetSize ${SecDGreek} 0
  ${endif}
  StrCpy $Search "et_EE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEstonian} $0
   SectionSetSize ${SecDEstonian} 0
  ${endif}
  StrCpy $Search "en_AU"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEnglishAU} $0
   SectionSetSize ${SecDEnglishAU} 0
  ${endif}
  StrCpy $Search "en_CA"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEnglishCA} $0
   SectionSetSize ${SecDEnglishCA} 0
  ${endif}
  StrCpy $Search "en_GB"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEnglishGB} $0
   SectionSetSize ${SecDEnglishGB} 0
  ${endif}
  StrCpy $Search "en_NZ"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEnglishNZ} $0
   SectionSetSize ${SecDEnglishNZ} 0
  ${endif}
  StrCpy $Search "en_US"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEnglishUS} $0
   SectionSetSize ${SecDEnglishUS} 0
  ${endif}
  StrCpy $Search "es_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSpanishES} $0
   SectionSetSize ${SecDSpanishES} 0
  ${endif}
  StrCpy $Search "es_MX"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSpanishMX} $0
   SectionSetSize ${SecDSpanishMX} 0
  ${endif}
  StrCpy $Search "eo_EO"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDEsperanto} $0
   SectionSetSize ${SecDEsperanto} 0
  ${endif}
  StrCpy $Search "eu_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDBasque} $0
   SectionSetSize ${SecDBasque} 0
  ${endif}
  StrCpy $Search "fa_IR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDFarsi} $0
   SectionSetSize ${SecDFarsi} 0
  ${endif}
  StrCpy $Search "fr_CA"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDFrenchCanada} $0
   SectionSetSize ${SecDFrenchCanada} 0
  ${endif}
  StrCpy $Search "fr_FR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDFrench} $0
   SectionSetSize ${SecDFrench} 0
  ${endif}
  StrCpy $Search "ga_IR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDGaelic} $0
   SectionSetSize ${SecDGaelic} 0
  ${endif}
  StrCpy $Search "gd_GB"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDScottish} $0
   SectionSetSize ${SecDScottish} 0
  ${endif}
  StrCpy $Search "gl_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDGalician} $0
   SectionSetSize ${SecDGalician} 0
  ${endif}
  StrCpy $Search "he_IL"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDHebrew} $0
   SectionSetSize ${SecDHebrew} 0
  ${endif}
  StrCpy $Search "hi_IN"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDHindi} $0
   SectionSetSize ${SecDHindi} 0
  ${endif}
  StrCpy $Search "hr_HR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDCroatian} $0
   SectionSetSize ${SecDCroatian} 0
  ${endif}
  StrCpy $Search "hu_HU"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDHungarian} $0
   SectionSetSize ${SecDHungarian} 0
  ${endif}
  StrCpy $Search "ia_IA"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDInterlingua} $0
   SectionSetSize ${SecDInterlingua} 0
  ${endif}
  StrCpy $Search "is_IS"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDIcelandic} $0
   SectionSetSize ${SecDIcelandic} 0
  ${endif}
  StrCpy $Search "it_IT"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDItalian} $0
   SectionSetSize ${SecDItalian} 0
  ${endif}
  StrCpy $Search "kk_KZ"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDKazakh} $0
   SectionSetSize ${SecDKazakh} 0
  ${endif}
  StrCpy $Search "ko_KR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDKorean} $0
   SectionSetSize ${SecDKorean} 0
  ${endif}
  StrCpy $Search "la_LA"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDLatin} $0
   SectionSetSize ${SecDLatin} 0
  ${endif}
  StrCpy $Search "lt_LT"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDLithuanian} $0
   SectionSetSize ${SecDLithuanian} 0
  ${endif}
  StrCpy $Search "lv_LV"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDLatvian} $0
   SectionSetSize ${SecDLatvian} 0
  ${endif}
  StrCpy $Search "mr_IN"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDMarathi} $0
   SectionSetSize ${SecDMarathi} 0
  ${endif}
  StrCpy $Search "nl_NL"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDDutch} $0
   SectionSetSize ${SecDDutch} 0
  ${endif}
  StrCpy $Search "nb_NO"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDNorwegianNB} $0
   SectionSetSize ${SecDNorwegianNB} 0
  ${endif}
  StrCpy $Search "nn_NO"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDNorwegianNN} $0
   SectionSetSize ${SecDNorwegianNN} 0
  ${endif}
  StrCpy $Search "oc_FR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDOccitan} $0
   SectionSetSize ${SecDOccitan} 0
  ${endif}
  StrCpy $Search "pl_PL"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDPolish} $0
   SectionSetSize ${SecDPolish} 0
  ${endif}
  StrCpy $Search "pt_BR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDPortugueseBR} $0
   SectionSetSize ${SecDPortugueseBR} 0
  ${endif}
  StrCpy $Search "pt_PT"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDPortuguesePT} $0
   SectionSetSize ${SecDPortuguesePT} 0
  ${endif}
  StrCpy $Search "ro_RO"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDRomanian} $0
   SectionSetSize ${SecDRomanian} 0
  ${endif}
  StrCpy $Search "ru_RU"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDRussian} $0
   SectionSetSize ${SecDRussian} 0
  ${endif}
  StrCpy $Search "dsb_DE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSorbianD} $0
   SectionSetSize ${SecDSorbianD} 0
  ${endif}
  StrCpy $Search "hsb_DE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSorbianH} $0
   SectionSetSize ${SecDSorbianH} 0
  ${endif}
  StrCpy $Search "sq_AL"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDAlbanian} $0
   SectionSetSize ${SecDAlbanian} 0
  ${endif}
  StrCpy $Search "sl_SI"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSlovenian} $0
   SectionSetSize ${SecDSlovenian} 0
  ${endif}
  StrCpy $Search "sk_SK"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSlovakian} $0
   SectionSetSize ${SecDSlovakian} 0
  ${endif}
  StrCpy $Search "sr_RS"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSerbianC} $0
   SectionSetSize ${SecDSerbianC} 0
  ${endif}
  StrCpy $Search "sr_RS-Latin"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSerbianL} $0
   SectionSetSize ${SecDSerbianL} 0
  ${endif}
  StrCpy $Search "sv_SE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDSwedish} $0
   SectionSetSize ${SecDSwedish} 0
  ${endif}
  StrCpy $Search "ta_IN"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDTamil} $0
   SectionSetSize ${SecDTamil} 0
  ${endif}
  StrCpy $Search "te_IN"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDTelugu} $0
   SectionSetSize ${SecDTelugu} 0
  ${endif}
  StrCpy $Search "th_TH"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDThai} $0
   SectionSetSize ${SecDThai} 0
  ${endif}
  StrCpy $Search "tk_TM"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDTurkmen} $0
   SectionSetSize ${SecDTurkmen} 0
  ${endif}
  StrCpy $Search "tr_TR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDTurkish} $0
   SectionSetSize ${SecDTurkish} 0
  ${endif}
  StrCpy $Search "uk_UA"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDUkrainian} $0
   SectionSetSize ${SecDUkrainian} 0
  ${endif}
  StrCpy $Search "ur_PK"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDUrdu} $0
   SectionSetSize ${SecDUrdu} 0
  ${endif}
  StrCpy $Search "vi_VN"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecDVietnamese} $0
   SectionSetSize ${SecDVietnamese} 0
  ${endif}
  
  # select sections of already installed thesaurus dictionaries, make them read-only
  # and set the necessary size to 0 bytes
  StrCpy $String $FoundThes
  StrCpy $Search "bg_BG"
  Call StrPoint # function from LyXUtils.nsh
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTBulgarian} $0
   SectionSetSize ${SecTBulgarian} 0
  ${endif}
  StrCpy $Search "ca_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTCatalan} $0
   SectionSetSize ${SecTCatalan} 0
  ${endif}
  StrCpy $Search "cs_CZ"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTCzech} $0
   SectionSetSize ${SecTCzech} 0
  ${endif}
  StrCpy $Search "da_DK"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTDanish} $0
   SectionSetSize ${SecTDanish} 0
  ${endif}
  StrCpy $Search "de_DE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTGermanDA} $0
   SectionSetSize ${SecTGermanDA} 0
  ${endif} 
  StrCpy $Search "de_CH"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTGermanCH} $0
   SectionSetSize ${SecTGermanCH} 0
  ${endif}
  StrCpy $Search "en_GB"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTEnglishGB} $0
   SectionSetSize ${SecTEnglishGB} 0
  ${endif} 
  StrCpy $Search "en_US"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTEnglishUSAU} $0
   SectionSetSize ${SecTEnglishUSAU} 0
  ${endif}
  StrCpy $Search "es_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTSpanish} $0
   SectionSetSize ${SecTSpanish} 0
  ${endif}
  StrCpy $Search "fr_FR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTFrench} $0
   SectionSetSize ${SecTFrench} 0
  ${endif}
  StrCpy $Search "ga_IR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTGaelic} $0
   SectionSetSize ${SecTGaelic} 0
  ${endif}
  StrCpy $Search "gl_ES"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTGalician} $0
   SectionSetSize ${SecTGalician} 0
  ${endif}
  StrCpy $Search "el_GR"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTGreek} $0
   SectionSetSize ${SecTGreek} 0
  ${endif}
  StrCpy $Search "is_IS"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTIcelandic} $0
   SectionSetSize ${SecTIcelandic} 0
  ${endif}
  StrCpy $Search "it_IT"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTItalian} $0
   SectionSetSize ${SecTItalian} 0
  ${endif}
  StrCpy $Search "hu_HU"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTHungarian} $0
   SectionSetSize ${SecTHungarian} 0
  ${endif}
  StrCpy $Search "nb_NO"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTNorwegianNB} $0
   SectionSetSize ${SecTNorwegianNB} 0
  ${endif}
  StrCpy $Search "pl_PL"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTPolish} $0
   SectionSetSize ${SecTPolish} 0
  ${endif}
  StrCpy $Search "pt_PT"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTPortuguese} $0
   SectionSetSize ${SecTPortuguese} 0
  ${endif}
  StrCpy $Search "ro_RO"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTRomanian} $0
   SectionSetSize ${SecTRomanian} 0
  ${endif}
  StrCpy $Search "ru_RU"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTRussian} $0
   SectionSetSize ${SecTRussian} 0
  ${endif}
  StrCpy $Search "sl_SI"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTSlovenian} $0
   SectionSetSize ${SecTSlovenian} 0
  ${endif}
  StrCpy $Search "sk_SK"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTSlovakian} $0
   SectionSetSize ${SecTSlovakian} 0
  ${endif}
  StrCpy $Search "sv_SE"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTSwedish} $0
   SectionSetSize ${SecTSwedish} 0
  ${endif}
  StrCpy $Search "uk_UA"
  Call StrPoint
  ${if} $Pointer != "-1"
   IntOp $0 ${SF_SELECTED} | ${SF_RO}
   SectionSetFlags ${SecTUkrainian} $0
   SectionSetSize ${SecTUkrainian} 0
  ${endif}
  
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
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)" /SD IDOK
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
   MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)" /SD IDOK
   Abort
  ${endif}
  # warning when LyX couldn't be found in the registry
  ${if} $0 == "" # check in HKCU
   ReadRegStr $0 HKCU "${APP_UNINST_KEY}" "DisplayVersion"
   ${if} $0 == ""
     MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)" /SD IDOK
   ${endif}
  ${endif}
  
  # Macro to investigate name of LyX's preferences folders to be able remove them
  !insertmacro UnAppPreSuff $AppPre $AppSuff # macro from LyXUtils.nsh

  # test if MiKTeX was installed together with LyX
  ReadRegStr $0 SHCTX "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${if} $0 == "Yes${APP_SERIES_KEY}"
   SectionSetText 2 "MiKTeX" # names the corersponding uninstaller section
   StrCpy $LaTeXInstalled "MiKTeX"
  ${else}
   SectionSetText 2 "" # hides the corresponding uninstaller section
  ${endif}
  
  # test if JabRef was installed together with LyX
  ReadRegStr $0 SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  ${if} $0 == "Yes${APP_SERIES_KEY}"
   SectionSetText 3 "JabRef" # names the corersponding uninstaller section
   StrCpy $JabRefInstalled "Yes"
  ${else}
   SectionSetText 3 "" # hides the corresponding uninstaller section
  ${endif}

  # question message if the user really wants to uninstall LyX
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" /SD IDYES IDYES +2 # continue if yes
  Abort

FunctionEnd

