#-------------------------------------------------------------
# - - - - - - - - - - - - Preamble - - - - - - - - - - - - - -
#-------------------------------------------------------------

  # ===== Variables =====

    Var lcID # containts the ID of the default Windows UI Language

# ================
# Helper Functions
# ================

# Forward Declarations

!macro DownloadHunspell langCode
  Push "${langCode}.aff"
  Call DownloadHunspell
  Push "${langCode}.dic"
  Call DownloadHunspell
!macroend

!define DownloadHunspell "!insertmacro DownloadHunspell"

!macro DownloadThesaurus langCode
  Push "th_${langCode}_v2.dat"
  Call DownloadThesaurus
  Push "th_${langCode}_v2.idx"
  Call DownloadThesaurus
!macroend

!define DownloadThesaurus "!insertmacro DownloadThesaurus"

!macro CheckHunspell sectionIdx langCode langID
  Push ${sectionIdx}
  Push ${langCode}
  Push ${langID}
  Call CheckHunspell
!macroend

!define CheckHunspell "!insertmacro CheckHunspell"

!macro CheckThesaurus sectionIdx langCode
  Push ${sectionIdx}
  Push ${langCode}
  Call CheckThesaurus
!macroend

!define CheckThesaurus "!insertmacro CheckThesaurus"

# Functions

Function DownloadHunspell
  Exch $R9 # "${langCode}.aff" / "${langCode}.dic"
  Push $9

  ${IfNot} ${FileExists} "$INSTDIR\Resources\dicts\$R9"
    inetc::get /TIMEOUT=5000 "https://www.lyx.org/trac/export/HEAD/lyxsvn/dictionaries/trunk/dicts/$R9" "$INSTDIR\Resources\dicts\$R9" /END
    Pop $9
    ${If} $9 != "OK"
      MessageBox MB_OK|MB_ICONEXCLAMATION "$(HunspellFailed)" # $(HunspellFailed) uses $R9
    ${EndIf}
  ${EndIf}

  Pop $9
  Pop $R9
FunctionEnd

Function DownloadThesaurus
  Exch $R9 # "th_${langCode}_v2.dat" / "th_${langCode}_v2.idx"
  Push $9

  ${IfNot} ${FileExists} "$INSTDIR\Resources\thes\$R9"
    inetc::get /TIMEOUT=5000 "https://www.lyx.org/trac/export/HEAD/lyxsvn/dictionaries/trunk/thes/$R9" "$INSTDIR\Resources\thes\$R9" /END
    Pop $9
    ${If} $9 != "OK"
      MessageBox MB_OK|MB_ICONEXCLAMATION "$(ThesaurusFailed)" # $(ThesaurusFailed) uses $R9
    ${EndIf}
  ${EndIf}

  Pop $9
  Pop $R9
FunctionEnd

Function CheckHunspell
  Exch $9 # ${langID}
  Exch
  Exch $8 # ${langCode}
  Exch 2
  Exch $7 # ${sectionIdx}

  ${If} ${FileExists} "$INSTDIR\Resources\dicts\$8.aff"
  ${OrIf} ${FileExists} "$INSTDIR\Resources\dicts\$8.dic"
    IntOp $8 ${SF_SELECTED} | ${SF_RO}
    SectionSetFlags $7 $8
    SectionSetSize $7 0
  ${ElseIf} $lcID == $9
    SectionSetFlags $7 ${SF_SELECTED}
  ${EndIf}

  Pop $7
  Pop $9
  Pop $8
FunctionEnd

Function CheckThesaurus
  Exch $9 # ${langCode}
  Exch
  Exch $8 # ${sectionIdx}

  ${If} ${FileExists} "$INSTDIR\Resources\thes\th_$9_v2.dat"
  ${OrIf} ${FileExists} "$INSTDIR\Resources\thes\th_$9_v2.idx"
    IntOp $9 ${SF_SELECTED} | ${SF_RO}
    SectionSetFlags $8 $9
    SectionSetSize $8 0
  ${EndIf}
  
  Pop $8
  Pop $9
FunctionEnd

# ========
# Sections
# ========

# Expand the list of dictionaries by default as this was requested by several
# users. For the thesaurus this is was not requested because this section
# is by default empty.
SectionGroup /e "$(SecDictionaries)" SecDictionaries

  Section /o "Afrikaans" SecDAfrikaans
    ${DownloadHunspell} "af_ZA"
    AddSize 1483
  SectionEnd

  Section /o "العربية" SecDArabic
    ${DownloadHunspell} "ar_SA"
    AddSize 34951
  SectionEnd

  Section /o "հայերեն" SecDArmenian
    ${DownloadHunspell} "hy_AM"
    AddSize 2060
  SectionEnd

  Section /o "Bahasa Indonesia" SecDIndonesian
    ${DownloadHunspell} "id_ID"
    AddSize 323
  SectionEnd

  Section /o "Bahasa Melayu" SecDMalayan
    ${DownloadHunspell} "ms_MY"
    AddSize 228
  SectionEnd

  Section /o "беларускі" SecDBelarusian 
    ${DownloadHunspell} "be_BY"
    AddSize 1692
  SectionEnd

  Section /o "Bosanski" SecDBosnian
    ${DownloadHunspell} "bs_BA"
    AddSize 350
  SectionEnd

  Section /o "Brezhoneg" SecDBreton 
    ${DownloadHunspell} "br_FR"
    AddSize 5427
  SectionEnd

  Section /o "български" SecDBulgarian
    ${DownloadHunspell} "bg_BG"
    AddSize 1587
  SectionEnd

  Section /o "Català" SecDCatalanian
    ${DownloadHunspell} "ca_ES"
    AddSize 2005
  SectionEnd

  Section /o "Ceština" SecDCzech
    ${DownloadHunspell} "cs_CZ"
    AddSize 2254
  SectionEnd

  Section /o "Coptic" SecDCoptic # Native name displayed not correctly in the installer for a strange reason
    ${DownloadHunspell} "cop_EG"
    AddSize 151
  SectionEnd

  Section /o "Cymraeg" SecDWelsh 
    ${DownloadHunspell} "cy_GB"
    AddSize 1578
  SectionEnd

  Section /o "Dansk" SecDDanish
    ${DownloadHunspell} "da_DK"
    AddSize 2895
  SectionEnd

  Section /o "Deutsch (alt)" SecDGermanAlt
    ${DownloadHunspell} "de-alt"
    AddSize 2572
  SectionEnd

  Section /o "Deutsch (A)" SecDGermanAT
    ${DownloadHunspell} "de_AT"
    AddSize 4277
  SectionEnd

  Section /o "Deutsch (CH)" SecDGermanCH
    ${DownloadHunspell} "de_CH"
    AddSize 4276
  SectionEnd

  Section "Deutsch (D)" SecDGermanD
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "de_DE"
    AddSize 4274
  SectionEnd

  Section /o "Ελληνικά" SecDGreek
    ${DownloadHunspell} "el_GR"
    AddSize 17865
  SectionEnd

  Section /o "Eesti" SecDEstonian
    ${DownloadHunspell} "et_EE"
    AddSize 4513
  SectionEnd

  Section /o "English (AU)" SecDEnglishAU
    ${DownloadHunspell} "en_AU"
    AddSize 542
  SectionEnd

  Section /o "English (CA)" SecDEnglishCA
    ${DownloadHunspell} "en_CA"
    AddSize 540
  SectionEnd

  Section "English (GB)" SecDEnglishGB
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "en_GB"
    AddSize 801
  SectionEnd

  Section /o "English (NZ)" SecDEnglishNZ
    ${DownloadHunspell} "en_NZ"
    AddSize 801
  SectionEnd

  Section "English (US)" SecDEnglishUS
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "en_US"
    AddSize 538
  SectionEnd

  Section "Español (ES)" SecDSpanishES
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "es_ES"
    AddSize 938
  SectionEnd

  Section "Español (MX)" SecDSpanishMX
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "es_MX"
    AddSize 938
  SectionEnd

  Section /o "Esperanto" SecDEsperanto
    ${DownloadHunspell} "eo_EO"
    AddSize 390
  SectionEnd

  Section /o "Euskara" SecDBasque
    ${DownloadHunspell} "eu_ES"
    AddSize 4970
  SectionEnd

  Section /o "فارسی" SecDFarsi
    ${DownloadHunspell} "fa_IR"
    AddSize 6879
  SectionEnd
  /* Language not supported by LyX
  Section /o "Føroyskt" SecDFaroese
    ${DownloadHunspell} "fo_FO"
    AddSize 1449
  SectionEnd */

  Section "Français" SecDFrench
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "fr_FR"
    AddSize 1314
  SectionEnd

  Section /o "Français (Canada)" SecDFrenchCanada
    ${DownloadHunspell} "fr_CA"
    AddSize 1314
  SectionEnd

  Section /o "Furlan" SecDFriulian
    ${DownloadHunspell} "fur_IT"
    AddSize 381
  SectionEnd

  Section /o "Gaeilge" SecDIrish
    ${DownloadHunspell} "ga_IE"
    AddSize 1330
  SectionEnd

  Section /o "Gàidhlig" SecDScottish
    ${DownloadHunspell} "gd_GB"
    AddSize 4704
  SectionEnd

  Section /o "Galego" SecDGalician
    ${DownloadHunspell} "gl_ES"
    AddSize 3222
  SectionEnd

  Section /o "Hrvatski" SecDCroatian
    ${DownloadHunspell} "hr_HR"
    AddSize 809
  SectionEnd

  Section /o "हिंदी" SecDHindi
    ${DownloadHunspell} "hi_IN"
    AddSize 298
  SectionEnd

  Section /o "Interlingua" SecDInterlingua
    ${DownloadHunspell} "ia_IA"
    AddSize 599
  SectionEnd

  Section /o "Íslenska" SecDIcelandic
    ${DownloadHunspell} "is_IS"
    AddSize 2700
  SectionEnd

  Section /o "Italiano" SecDItalian
    ${DownloadHunspell} "it_IT"
    AddSize 1340
  SectionEnd

  Section /o "עִברִית" SecDHebrew
    ${DownloadHunspell} "he_IL"
    AddSize 7698
  SectionEnd
  /* dicts missing
  Section /o "ქართული" SecDGeorgian
    ${DownloadHunspell} "ka_GE"
    AddSize 3952
  SectionEnd */

  Section /o "Қазақша" SecDKazakh
    ${DownloadHunspell} "kk_KZ"
    AddSize 2182
  SectionEnd

  Section /o "ភាសាខ្មែរ" SecDKhmer
    ${DownloadHunspell} "km_KH"
    AddSize 2093
  SectionEnd

  Section /o "한국어" SecDKorean
    ${DownloadHunspell} "ko_KR"
    AddSize 15586
  SectionEnd
  /* Language not supported by LyX
  Section "Kreyòl Ayisyen" SecDHaitianCreole
    ${DownloadHunspell} "ht_HT"
    AddSize 118
  SectionEnd */

  Section /o "kurdî" SecDKurdishL
    ${DownloadHunspell} "kmr"
    AddSize 48
  SectionEnd
  /* Language not supported by LyX
  Section /o "kurdо" SecDKurdishT
    ${DownloadHunspell} "ku_TR"
    AddSize 47
  SectionEnd */

  Section /o "Latina" SecDLatin
    ${DownloadHunspell} "la_LA"
    AddSize 1291
  SectionEnd

  Section /o "Lietuvių" SecDLithuanian
    ${DownloadHunspell} "lt_LT"
    AddSize 1151
  SectionEnd

  Section /o "Latviešu" SecDLatvian
    ${DownloadHunspell} "lv_LV"
    AddSize 2243
  SectionEnd

  Section /o "Mакедонски" SecDMacedonian
    ${DownloadHunspell} "mk_MK"
    AddSize 2862
  SectionEnd

  Section /o "Magyar" SecDHungarian
    ${DownloadHunspell} "hu_HU"
    AddSize 3672
  SectionEnd

  Section /o "मराठी" SecDMarathi
    ${DownloadHunspell} "mr_IN"
    AddSize 5291
  SectionEnd

  Section /o "Nederlands" SecDDutch
    ${DownloadHunspell} "nl_NL"
    AddSize 2393
  SectionEnd

  Section /o "Norsk (Bokmål)" SecDNorwegianNB
    ${DownloadHunspell} "nb_NO"
    AddSize 3992
  SectionEnd

  Section /o "Norsk (Nynorsk)" SecDNorwegianNN
    ${DownloadHunspell} "nn_NO"
    AddSize 2982
  SectionEnd

  Section /o "Occitan" SecDOccitan
    ${DownloadHunspell} "oc_FR"
    AddSize 684
  SectionEnd

  Section /o "ພາສາລາວ" SecDLao
    ${DownloadHunspell} "lo_LA"
    AddSize 200
  SectionEnd

  Section /o "Polski" SecDPolish
    ${DownloadHunspell} "pl_PL"
    AddSize 4675
  SectionEnd

  Section /o "Português (BR)" SecDPortugueseBR
    ${DownloadHunspell} "pt_BR"
    AddSize 5510
  SectionEnd

  Section "Português (PT)" SecDPortuguesePT
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "pt_PT"
    AddSize 1532
  SectionEnd

  Section /o "Româna" SecDRomanian
    ${DownloadHunspell} "ro_RO"
    AddSize 2199
  SectionEnd

  Section "Русский" SecDRussian
    # already installed by default
    SectionIn RO
    #${DownloadHunspell} "ru_RU"
    AddSize 1976
  SectionEnd

  Section /o "serbšćina (Dolno)" SecDSorbianD
    ${DownloadHunspell} "dsb_DE"
    AddSize 906
  SectionEnd

  Section /o "serbšćina (Horno)" SecDSorbianH
    ${DownloadHunspell} "hsb_DE"
    AddSize 741
  SectionEnd

  Section /o "Shqip" SecDAlbanian
    ${DownloadHunspell} "sq_AL"
    AddSize 2553
  SectionEnd

  Section /o "Slovenščina" SecDSlovenian
    ${DownloadHunspell} "sl_SI"
    AddSize 2914
  SectionEnd

  Section /o "Slovenčina" SecDSlovakian
    ${DownloadHunspell} "sk_SK"
    AddSize 3311
  SectionEnd

  Section /o "Српски (Ћирилица)" SecDSerbianC
    ${DownloadHunspell} "sr_RS"
    AddSize 3559
  SectionEnd

  Section /o "Srpski (Latinica)" SecDSerbianL
    ${DownloadHunspell} "sr_RS-Latin"
    AddSize 1997
  SectionEnd

  Section /o "Svenska" SecDSwedish
    ${DownloadHunspell} "sv_SE"
    AddSize 2287
  SectionEnd

  Section /o "தமிழ்" SecDTamil
    ${DownloadHunspell} "ta_IN"
    AddSize 5809
  SectionEnd

  Section /o "తెలుగు" SecDTelugu
    ${DownloadHunspell} "te_IN"
    AddSize 3324
  SectionEnd
  
  Section /o "ไทย" SecDThai
    ${DownloadHunspell} "th_TH"
    AddSize 342
  SectionEnd

  Section /o "དབུས་སྐད་" SecDTibetan
    ${DownloadHunspell} "bo_CN"
    AddSize 7
  SectionEnd

  Section /o "Tiếng Việt" SecDVietnamese
    ${DownloadHunspell} "vi_VN"
    AddSize 40
  SectionEnd

  Section /o "Türkmençe" SecDTurkmen
    ${DownloadHunspell} "tk_TM"
    AddSize 928
  SectionEnd

  Section /o "Türkçe" SecDTurkish
    ${DownloadHunspell} "tr_TR"
    AddSize 9140
  SectionEnd

  Section /o "Українська" SecDUkrainian
    ${DownloadHunspell} "uk_UA"
    AddSize 2680
  SectionEnd

  Section /o "اردو" SecDUrdu
    ${DownloadHunspell} "ur_PK"
    AddSize 1379
  SectionEnd

SectionGroupEnd

SectionGroup "$(SecThesaurus)" SecThesaurus

  Section /o "العربية" SecTArabic
    ${DownloadThesaurus} "ar_SA"
    AddSize 761
  SectionEnd

  Section /o "Bahasa Indonesia" SecTIndonesian
    ${DownloadThesaurus} "id_ID"
    AddSize 2043
  SectionEnd

  Section /o "български" SecTBulgarian
    ${DownloadThesaurus} "bg_BG"
    AddSize 5313
  SectionEnd

  Section /o "Català" SecTCatalanian
    ${DownloadThesaurus} "ca_ES"
    AddSize 733
  SectionEnd

  Section /o "Ceština" SecTCzech
    ${DownloadThesaurus} "cs_CZ"
    AddSize 4723
  SectionEnd

  Section /o "Dansk" SecTDanish
    ${DownloadThesaurus} "da_DK"
    AddSize 4228
  SectionEnd

  Section /o "Deutsch (A)" SecTGermanAT
    ${DownloadThesaurus} "de_AT"
    AddSize 33721
  SectionEnd

  Section /o "Deutsch (CH)" SecTGermanCH
    ${DownloadThesaurus} "de_CH"
    AddSize 33721
  SectionEnd

  Section /o "Deutsch (D/A)" SecTGermanD
    ${DownloadThesaurus} "de_DE"
    AddSize 33721
  SectionEnd

  Section /o "Ελληνικά" SecTGreek
    ${DownloadThesaurus} "el_GR"
    AddSize 1795
  SectionEnd

  Section /o "English (AU)" SecTEnglishAU
    ${DownloadThesaurus} "en_AU"
    AddSize 21093
  SectionEnd

  Section /o "English (CA)" SecTEnglishCA
    ${DownloadThesaurus} "en_CA"
    AddSize 21093
  SectionEnd

  Section /o "English (GB)" SecTEnglishGB
    ${DownloadThesaurus} "en_GB"
    AddSize 21093
  SectionEnd

  Section /o "English (NZ)" SecTEnglishNZ
    ${DownloadThesaurus} "en_NZ"
    AddSize 21093
  SectionEnd

  Section /o "English (US)" SecTEnglishUS
    ${DownloadThesaurus} "en_US"
    AddSize 21093
  SectionEnd

  Section /o "Español" SecTSpanishES
    ${DownloadThesaurus} "es_ES"
    AddSize 3172
  SectionEnd

  Section /o "Español (MX)" SecTSpanishMX
    ${DownloadThesaurus} "es_MX"
    AddSize 3172
  SectionEnd

  Section /o "Français" SecTFrench
    ${DownloadThesaurus} "fr_FR"
    AddSize 5192
  SectionEnd

  Section /o "Français (Canada)" SecTFrenchCanada
    ${DownloadThesaurus} "fr_CA"
    AddSize 5192
  SectionEnd

  Section /o "Gaeilge" SecTIrish
    ${DownloadThesaurus} "ga_IE"
    AddSize 35989
  SectionEnd

  Section /o "Galego" SecTGalician
    ${DownloadThesaurus} "gl_ES"
    AddSize 499
  SectionEnd

  Section /o "Íslenska" SecTIcelandic
    ${DownloadThesaurus} "is_IS"
    AddSize 170
  SectionEnd

  Section /o "Italiano" SecTItalian
    ${DownloadThesaurus} "it_IT"
    AddSize 2521
  SectionEnd

  Section /o "Latviešu" SecTLatvian
    ${DownloadThesaurus} "lv_LV"
    AddSize 2
  SectionEnd

  Section /o "Magyar" SecTHungarian
    ${DownloadThesaurus} "hu_HU"
    AddSize 2473
  SectionEnd

  Section /o "Nederlands" SecTDutch
    ${DownloadThesaurus} "nl_NL"
    AddSize 6641
  SectionEnd
  /* Language not supported by LyX
  Section /o "नेपाली" SecTNepali
    ${DownloadThesaurus} "ne_NP"
    AddSize 967
  SectionEnd */

  Section /o "Norsk (Bokmål)" SecTNorwegianNB
    ${DownloadThesaurus} "nb_NO"
    AddSize 2535
  SectionEnd

  Section /o "Norsk (Nynorsk)" SecTNorwegianNN
    ${DownloadThesaurus} "nn_NO"
    AddSize 2
  SectionEnd

  Section /o "Polski" SecTPolish
    ${DownloadThesaurus} "pl_PL"
    AddSize 3088
  SectionEnd

  Section /o "Português" SecTPortuguesePT
    ${DownloadThesaurus} "pt_PT"
    AddSize 3858
  SectionEnd

  Section /o "Româna" SecTRomanian
    ${DownloadThesaurus} "ro_RO"
    AddSize 3647
  SectionEnd

  Section /o "Русский" SecTRussian
    ${DownloadThesaurus} "ru_RU"
    AddSize 2142
  SectionEnd

  Section /o "Slovenšcina" SecTSlovenian
    ${DownloadThesaurus} "sl_SI"
    AddSize 1257
  SectionEnd

  Section /o "Slovenčina" SecTSlovakian
    ${DownloadThesaurus} "sk_SK"
    AddSize 928
  SectionEnd

  Section /o "Svenska" SecTSwedish
    ${DownloadThesaurus} "sv_SE"
    AddSize 737
  SectionEnd

  Section /o "Українська" SecTUkrainian
    ${DownloadThesaurus} "uk_UA"
    AddSize 1309
  SectionEnd

SectionGroupEnd

# Custom function, called before entering the components page
# checks the directory for already existent dictionaries and marks the corresponding sections above
# also activates the section to download dictionary for the current UI language
Function CheckDictionaries

  System::Call "kernel32::GetUserDefaultUILanguage() i.s"
  Pop $lcID

  # Check Hunspell dictionaries           language code    language ID (HHH means does not exis)
  ${CheckHunspell} ${SecDAfrikaans}       "af_ZA"          "1078"
  ${CheckHunspell} ${SecDArabic}          "ar_SA"          "1025"
  ${CheckHunspell} ${SecDArmenian}        "hy_AM"          "1067"
  ${CheckHunspell} ${SecDIndonesian}      "id_ID"          "1057"
  ${CheckHunspell} ${SecDMalayan}         "ms_MY"          "1086"
  ${CheckHunspell} ${SecDBelarusian}      "be_BY"          "1059"
  ${CheckHunspell} ${SecDBosnian}         "bs_BA"          "5146"
  ${CheckHunspell} ${SecDBreton}          "br_FR"          "1150"
  ${CheckHunspell} ${SecDBulgarian}       "bg_BG"          "1026"
  ${CheckHunspell} ${SecDCatalanian}      "ca_ES"          "1027"
  ${CheckHunspell} ${SecDCzech}           "cs_CZ"          "1029"
  ${CheckHunspell} ${SecDCoptic}          "cop_EG"         "HHH1"
  ${CheckHunspell} ${SecDWelsh}           "cy_GB"          "1106"
  ${CheckHunspell} ${SecDDanish}          "da_DK"          "1030"
  ${CheckHunspell} ${SecDGermanAlt}       "de-alt"         "HHH2"
  ${CheckHunspell} ${SecDGermanAT}        "de_AT"          "3079"
  ${CheckHunspell} ${SecDGermanCH}        "de_CH"          "2055"
 #${CheckHunspell} ${SecDGermanD}         "de_DE"          "1031" # Installed by default
  ${CheckHunspell} ${SecDGreek}           "el_GR"          "1032"
  ${CheckHunspell} ${SecDEstonian}        "et_EE"          "1061"
  ${CheckHunspell} ${SecDEnglishAU}       "en_AU"          "3081"
  ${CheckHunspell} ${SecDEnglishCA}       "en_CA"          "4105"
 #${CheckHunspell} ${SecDEnglishGB}       "en_GB"          "2057" # Installed by default
  ${CheckHunspell} ${SecDEnglishNZ}       "en_NZ"          "5129"
 #${CheckHunspell} ${SecDEnglishUS}       "en_US"          "1033" # Installed by default
 #${CheckHunspell} ${SecDSpanishES}       "es_ES"          "1034" # Installed by default
 #${CheckHunspell} ${SecDSpanishMX}       "es_MX"          "2058" # Installed by default
  ${CheckHunspell} ${SecDEsperanto}       "eo_EO"          "HHH3"
  ${CheckHunspell} ${SecDBasque}          "eu_ES"          "1069"
  ${CheckHunspell} ${SecDFarsi}           "fa_IR"          "1065"
 #${CheckHunspell} ${SecDFaroese}         "fo_FO"          "1080" # Not supported
 #${CheckHunspell} ${SecDFrench}          "fr_FR"          "1036" # Installed by default
  ${CheckHunspell} ${SecDFrenchCanada}    "fr_CA"          "3084"
  ${CheckHunspell} ${SecDFriulian}        "fur_IT"         "HHH4"
  ${CheckHunspell} ${SecDIrish}           "ga_IE"          "2108"
  ${CheckHunspell} ${SecDScottish}        "gd_GB"          "1084"
  ${CheckHunspell} ${SecDGalician}        "gl_ES"          "1110"
  ${CheckHunspell} ${SecDCroatian}        "hr_HR"          "1050"
  ${CheckHunspell} ${SecDHindi}           "hi_IN"          "1081"
  ${CheckHunspell} ${SecDInterlingua}     "ia_IA"          "HHH5"
  ${CheckHunspell} ${SecDIcelandic}       "is_IS"          "1039"
  ${CheckHunspell} ${SecDItalian}         "it_IT"          "1040"
  ${CheckHunspell} ${SecDHebrew}          "he_IL"          "1037"
 #${CheckHunspell} ${SecDGeorgian}        "ka_GE"          "1079" # Missing
  ${CheckHunspell} ${SecDKazakh}          "kk_KZ"          "1087"
  ${CheckHunspell} ${SecDKhmer}           "km_KH"          "1107"
  ${CheckHunspell} ${SecDKorean}          "ko_KR"          "1042"
 #${CheckHunspell} ${SecDHaitianCreole}   "ht_HT"          "HHH6" # Not supported
  ${CheckHunspell} ${SecDKurdishL}        "kmr"            "HHH7"
 #${CheckHunspell} ${SecDKurdishT}        "ku_TR"          "HHH8" # Not supported
  ${CheckHunspell} ${SecDLatin}           "la_LA"          "1142"
  ${CheckHunspell} ${SecDLithuanian}      "lt_LT"          "1063"
  ${CheckHunspell} ${SecDLatvian}         "lv_LV"          "1062"
  ${CheckHunspell} ${SecDMacedonian}      "mk_MK"          "1071"
  ${CheckHunspell} ${SecDHungarian}       "hu_HU"          "1038"
  ${CheckHunspell} ${SecDMarathi}         "mr_IN"          "1102"
  ${CheckHunspell} ${SecDDutch}           "nl_NL"          "1043"
  ${CheckHunspell} ${SecDNorwegianNB}     "nb_NO"          "1044"
  ${CheckHunspell} ${SecDNorwegianNN}     "nn_NO"          "2068"
  ${CheckHunspell} ${SecDOccitan}         "oc_FR"          "HHH9"
  ${CheckHunspell} ${SecDLao}             "lo_LA"          "1108"
  ${CheckHunspell} ${SecDPolish}          "pl_PL"          "1045"
  ${CheckHunspell} ${SecDPortugueseBR}    "pt_BR"          "1046"
 #${CheckHunspell} ${SecDPortuguesePT}    "pt_PT"          "2070" # Installed by default
  ${CheckHunspell} ${SecDRomanian}        "ro_RO"          "1048"
 #${CheckHunspell} ${SecDRussian}         "ru_RU"          "1049" # Installed by default
  ${CheckHunspell} ${SecDSorbianD}        "dsb_DE"         "HHH0"
  ${CheckHunspell} ${SecDSorbianH}        "hsb_DE"         "HHH1"
  ${CheckHunspell} ${SecDAlbanian}        "sq_AL"          "1052"
  ${CheckHunspell} ${SecDSlovenian}       "sl_SI"          "1060"
  ${CheckHunspell} ${SecDSlovakian}       "sk_SK"          "1051"
  ${CheckHunspell} ${SecDSerbianC}        "sr_RS"          "3098"
  ${CheckHunspell} ${SecDSerbianL}        "sr_RS-Latin"    "2074"
  ${CheckHunspell} ${SecDSwedish}         "sv_SE"          "1053"
  ${CheckHunspell} ${SecDTamil}           "ta_IN"          "1097"
  ${CheckHunspell} ${SecDTelugu}          "te_IN"          "1098"
  ${CheckHunspell} ${SecDThai}            "th_TH"          "1054"
  ${CheckHunspell} ${SecDTibetan}         "bo_CN"          "1105"
  ${CheckHunspell} ${SecDVietnamese}      "vi_VN"          "1066"
  ${CheckHunspell} ${SecDTurkmen}         "tk_TM"          "1090"
  ${CheckHunspell} ${SecDTurkish}         "tr_TR"          "1055"
  ${CheckHunspell} ${SecDUkrainian}       "uk_UA"          "1058"
  ${CheckHunspell} ${SecDUrdu}            "ur_PK"          "1056"

  # Check Thesaurus dictionaries
  ${CheckThesaurus} ${SecTArabic}           "ar_SA"
  ${CheckThesaurus} ${SecTIndonesian}       "id_ID"
  ${CheckThesaurus} ${SecTBulgarian}        "bg_BG"
  ${CheckThesaurus} ${SecTCatalanian}       "ca_ES"
  ${CheckThesaurus} ${SecTCzech}            "cs_CZ"
  ${CheckThesaurus} ${SecTDanish}           "da_DK"
  ${CheckThesaurus} ${SecTGermanAT}         "de_AT"
  ${CheckThesaurus} ${SecTGermanCH}         "de_CH"
  ${CheckThesaurus} ${SecTGermanD}          "de_DE"
  ${CheckThesaurus} ${SecTGreek}            "el_GR"
  ${CheckThesaurus} ${SecTEnglishAU}        "en_AU"
  ${CheckThesaurus} ${SecTEnglishCA}        "en_CA"
  ${CheckThesaurus} ${SecTEnglishGB}        "en_GB"
  ${CheckThesaurus} ${SecTEnglishNZ}        "en_NZ"
  ${CheckThesaurus} ${SecTEnglishUS}        "en_US"
  ${CheckThesaurus} ${SecTSpanishES}        "es_ES"
  ${CheckThesaurus} ${SecTSpanishMX}        "es_MX"
  ${CheckThesaurus} ${SecTFrench}           "fr_FR"
  ${CheckThesaurus} ${SecTFrenchCanada}     "fr_CA"
  ${CheckThesaurus} ${SecTIrish}            "ga_IE"
  ${CheckThesaurus} ${SecTGalician}         "gl_ES"
  ${CheckThesaurus} ${SecTIcelandic}        "is_IS"
  ${CheckThesaurus} ${SecTItalian}          "it_IT"
  ${CheckThesaurus} ${SecTLatvian}          "lv_LV"
  ${CheckThesaurus} ${SecTHungarian}        "hu_HU"
  ${CheckThesaurus} ${SecTDutch}            "nl_NL"
 #${CheckThesaurus} ${SecTNepali}           "ne_NP" # not supported
  ${CheckThesaurus} ${SecTNorwegianNB}      "nb_NO"
  ${CheckThesaurus} ${SecTNorwegianNN}      "nn_NO"
  ${CheckThesaurus} ${SecTPolish}           "pl_PL"
  ${CheckThesaurus} ${SecTPortuguesePT}     "pt_PT"
  ${CheckThesaurus} ${SecTRomanian}         "ro_RO"
  ${CheckThesaurus} ${SecTRussian}          "ru_RU"
  ${CheckThesaurus} ${SecTSlovenian}        "sl_SI"
  ${CheckThesaurus} ${SecTSlovakian}        "sk_SK"
  ${CheckThesaurus} ${SecTSwedish}          "sv_SE"
  ${CheckThesaurus} ${SecTUkrainian}        "uk_UA"

FunctionEnd