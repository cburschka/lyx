# Language settings for the installer
# create language code for the Windows system language

# LangNme is the language name when there exists a LyX translation for this language; used for the menu language dialog and for the Aspell installation
# LangNmeSys is the Windows system language; used for the Aspell installation
# LangCdeSys is the ISO 630 language code of the language; used for the Aspell installation
# LangSysEnc is the Windows codepage for the character encoding of the language; can be used in lyx.bat to assure that scripts are correctly executed
!macro TranslateLangCode LangNme LangNmeSys LangCdeSys LangISOCode LangSysEnc

 StrCpy ${LangCdeSys} ""

  ${if} ${LangISOCode} = 1025
   StrCpy ${LangNme} "Arabic"
   StrCpy ${LangCdeSys} "ar"
   StrCpy ${LangNmeSys} "Arabic"
   StrCpy ${LangSysEnc} "1256"
  ${endif}

  ${if} ${LangISOCode} = 1026
   StrCpy ${LangCdeSys} "bg"
   StrCpy ${LangNmeSys} "Bulgarian"
   StrCpy ${LangSysEnc} "1251"
  ${endif}

  ${if} ${LangISOCode} = 1027
   StrCpy ${LangCdeSys} "ca"
   StrCpy ${LangNmeSys} "Català"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

# currently no dictionary available
  ${if} ${LangISOCode} = 1028
   StrCpy ${LangNme} "Chinese (Taiwan)"
   StrCpy ${LangCdeSys} "zh_TW"
   StrCpy ${LangNmeSys} "Chinese"
   StrCpy ${LangSysEnc} "936"
  ${endif}

  ${if} ${LangISOCode} = 1029
   StrCpy ${LangNme} "Ceština"
   StrCpy ${LangCdeSys} "cs"
   StrCpy ${LangNmeSys} "Ceština"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 1030
   StrCpy ${LangNme} "Dansk"
   StrCpy ${LangCdeSys} "da"
   StrCpy ${LangNmeSys} "Dansk"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1031
   StrCpy ${LangNme} "Deutsch"
   StrCpy ${LangCdeSys} "de"
   StrCpy ${LangNmeSys} "Deutsch"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1032
   StrCpy ${LangCdeSys} "el"
   StrCpy ${LangNmeSys} "Greek"
   StrCpy ${LangSysEnc} "1253"
  ${endif}

  ${if} ${LangISOCode} = 1033
   StrCpy ${LangNme} "English"
   StrCpy ${LangCdeSys} "en"
   StrCpy ${LangNmeSys} "English"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1034
   StrCpy ${LangNme} "Español"
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1035
   StrCpy ${LangNme} "Suomi"
   StrCpy ${LangCdeSys} "fi"
   StrCpy ${LangNmeSys} "Suomi"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1036
   StrCpy ${LangNme} "Français"
   StrCpy ${LangCdeSys} "fr"
   StrCpy ${LangNmeSys} "Français"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1037
   StrCpy ${LangNme} "Hebrew"
   StrCpy ${LangCdeSys} "he"
   StrCpy ${LangNmeSys} "Hebrew"
   StrCpy ${LangSysEnc} "1255"
  ${endif}

  ${if} ${LangISOCode} = 1038
   StrCpy ${LangNme} "Magyar"
   StrCpy ${LangCdeSys} "hu"
   StrCpy ${LangNmeSys} "Magyar"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 1039
   StrCpy ${LangCdeSys} "is"
   StrCpy ${LangNmeSys} "Íslenska"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1040
   StrCpy ${LangNme} "Italiano"
   StrCpy ${LangCdeSys} "it"
   StrCpy ${LangNmeSys} "Italiano"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  # currently no dictionary available
  ${if} ${LangISOCode} = 1041
   StrCpy ${LangNme} "Japanese"
   StrCpy ${LangCdeSys} "ja"
   StrCpy ${LangNmeSys} "Japanese"
   StrCpy ${LangSysEnc} "932"
  ${endif}

  # currently no dictionary available
  ${if} ${LangISOCode} = 1042
   StrCpy ${LangNme} "Korean"
   StrCpy ${LangCdeSys} "ko"
   StrCpy ${LangNmeSys} "Korean"
   StrCpy ${LangSysEnc} "949"
  ${endif}

  ${if} ${LangISOCode} = 1043
   StrCpy ${LangNme} "Nederlands"
   StrCpy ${LangCdeSys} "nl"
   StrCpy ${LangNmeSys} "Nederlands"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1044
   StrCpy ${LangNme} "Norsk"
   StrCpy ${LangCdeSys} "no"
   StrCpy ${LangNmeSys} "Norsk"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1045
   StrCpy ${LangNme} "Polski"
   StrCpy ${LangCdeSys} "pl"
   StrCpy ${LangNmeSys} "Polski"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 1046  # for Portuguese (brasilian)
   StrCpy ${LangNme} "Português"
   StrCpy ${LangCdeSys} "pt"
   StrCpy ${LangNmeSys} "Português"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1048
   StrCpy ${LangNme} "Româna"
   StrCpy ${LangCdeSys} "ro"
   StrCpy ${LangNmeSys} "Româna"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 1049
   StrCpy ${LangNme} "Russian"
   StrCpy ${LangCdeSys} "ru"
   StrCpy ${LangNmeSys} "Russian"
   StrCpy ${LangSysEnc} "1251"
  ${endif}

  ${if} ${LangISOCode} = 1050
   StrCpy ${LangCdeSys} "hr"
   StrCpy ${LangNmeSys} "Hrvatski"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 1051
   StrCpy ${LangNme} "Slovenský"
   StrCpy ${LangCdeSys} "sk"
   StrCpy ${LangNmeSys} "Slovenský"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

# currently no dictionary available
#  ${if} ${LangISOCode} = 1052
#   StrCpy ${LangCdeSys} "sq"
#   StrCpy ${LangNmeSys} "Shqip"
#   StrCpy ${LangSysEnc} "1252"
#  ${endif}

  ${if} ${LangISOCode} = 1053
   StrCpy ${LangNme} "Svenska"
   StrCpy ${LangCdeSys} "sv"
   StrCpy ${LangNmeSys} "Svenska"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

# currently no dictionary available
#  ${if} ${LangISOCode} = 1054
#   StrCpy ${LangCdeSys} "th"
#   StrCpy ${LangNmeSys} "Thai"
#   StrCpy ${LangSysEnc} "874"
#  ${endif}

  ${if} ${LangISOCode} = 1055
   StrCpy ${LangNme} "Türkçe"
   StrCpy ${LangCdeSys} "tr"
   StrCpy ${LangNmeSys} "Türkçe"
   StrCpy ${LangSysEnc} "1254"
  ${endif}

  ${if} ${LangISOCode} = 1057
   StrCpy ${LangCdeSys} "id"
   StrCpy ${LangNmeSys} "Indonesian"
  ${endif}

  ${if} ${LangISOCode} = 1058
   StrCpy ${LangNme} "Ukrainian"
   StrCpy ${LangCdeSys} "uk"
   StrCpy ${LangNmeSys} "Ukrainian"
   StrCpy ${LangSysEnc} "1251"
  ${endif}

  ${if} ${LangISOCode} = 1060
   StrCpy ${LangNme} "Slovenšcina"
   StrCpy ${LangCdeSys} "sl"
   StrCpy ${LangNmeSys} "Slovenšcina"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 1061
   StrCpy ${LangCdeSys} "et"
   StrCpy ${LangNmeSys} "Eesti"
   StrCpy ${LangSysEnc} "1257"
  ${endif}

  ${if} ${LangISOCode} = 1062
   StrCpy ${LangCdeSys} "lv"
   StrCpy ${LangNmeSys} "Latviešu"
   StrCpy ${LangSysEnc} "1257"
  ${endif}

  ${if} ${LangISOCode} = 1063
   StrCpy ${LangCdeSys} "lt"
   StrCpy ${LangNmeSys} "Lietuva"
   StrCpy ${LangSysEnc} "1257"
  ${endif}
  
  ${if} ${LangISOCode} = 1065
   StrCpy ${LangCdeSys} "fa"
   StrCpy ${LangNmeSys} "Farsi"
  ${endif} 	
  
  ${if} ${LangISOCode} = 1067
   StrCpy ${LangCdeSys} "hy"
   StrCpy ${LangNmeSys} "Armenian"
  ${endif} 	

  # currently no dictionary available
  ${if} ${LangISOCode} = 1069
   StrCpy ${LangNme} "Euskara"
   StrCpy ${LangCdeSys} "eu"
   StrCpy ${LangNmeSys} "Euskara"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1071
   StrCpy ${LangCdeSys} "mk"
   StrCpy ${LangNmeSys} "Macedonian"
  ${endif}

  ${if} ${LangISOCode} = 1078
   StrCpy ${LangCdeSys} "af"
   StrCpy ${LangNmeSys} "Afrikaans"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 1081
   StrCpy ${LangCdeSys} "hi"
   StrCpy ${LangNmeSys} "Hindi"
  ${endif}

  ${if} ${LangISOCode} = 1086
   StrCpy ${LangCdeSys} "ms"
   StrCpy ${LangNmeSys} "Malay"
  ${endif}

  ${if} ${LangISOCode} = 1089
   StrCpy ${LangCdeSys} "sw"
   StrCpy ${LangNmeSys} "Swahili"
  ${endif}

  ${if} ${LangISOCode} = 1094
   StrCpy ${LangCdeSys} "pa"
   StrCpy ${LangNmeSys} "Punjabi"
  ${endif}

  ${if} ${LangISOCode} = 1095
   StrCpy ${LangCdeSys} "gu"
   StrCpy ${LangNmeSys} "Gujarati"
  ${endif}

  ${if} ${LangISOCode} = 1097
   StrCpy ${LangCdeSys} "ta"
   StrCpy ${LangNmeSys} "Tamil"
  ${endif}

  ${if} ${LangISOCode} = 1098
   StrCpy ${LangCdeSys} "te"
   StrCpy ${LangNmeSys} "Telugu"
  ${endif}

  ${if} ${LangISOCode} = 1102
   StrCpy ${LangCdeSys} "mr"
   StrCpy ${LangNmeSys} "Marathi"
  ${endif}

# currently no dictionary available  
#  ${if} ${LangISOCode} = 1103
#   StrCpy ${LangCdeSys} "sa"
#   StrCpy ${LangNmeSys} "Sanskrit"
#  ${endif}

  ${if} ${LangISOCode} = 1110
   StrCpy ${LangNme} "Galego"
   StrCpy ${LangCdeSys} "gl"
   StrCpy ${LangNmeSys} "Galego"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

# currently no dictionary available
  ${if} ${LangISOCode} = 2052
   StrCpy ${LangNme} "Chinese (China)"
   StrCpy ${LangCdeSys} "zh_CN"
   StrCpy ${LangNmeSys} "Chinese"
   StrCpy ${LangSysEnc} "936"
  ${endif}

  ${if} ${LangISOCode} = 2055
   StrCpy ${LangCdeSys} "de"
   StrCpy ${LangNmeSys} "Deutsch"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2057
   StrCpy ${LangCdeSys} "en"
   StrCpy ${LangNmeSys} "English"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2058
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2060
   StrCpy ${LangCdeSys} "fr"
   StrCpy ${LangNmeSys} "Français"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2064
   StrCpy ${LangCdeSys} "it"
   StrCpy ${LangNmeSys} "Italiano"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2067
   StrCpy ${LangCdeSys} "nl"
   StrCpy ${LangNmeSys} "Nederlands"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2068
   StrCpy ${LangNme} "Nynorsk"
   StrCpy ${LangCdeSys} "nn"
   StrCpy ${LangNmeSys} "Nynorsk"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2070  # for Portuguese
   StrCpy ${LangNme} "Português"
   StrCpy ${LangCdeSys} "pt"
   StrCpy ${LangNmeSys} "Português"
   StrCpy ${LangSysEnc} "1252"
  ${endif}  

  ${if} ${LangISOCode} = 2074
   StrCpy ${LangCdeSys} "sr"
   StrCpy ${LangNmeSys} "Srpski"
   StrCpy ${LangSysEnc} "1250"
  ${endif}

  ${if} ${LangISOCode} = 2077
   StrCpy ${LangCdeSys} "sv"
   StrCpy ${LangNmeSys} "Svenska"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 2110
   StrCpy ${LangCdeSys} "ms"
   StrCpy ${LangNmeSys} "Malay"
  ${endif}

# currently no dictionary available
  ${if} ${LangISOCode} = 3076
   StrCpy ${LangNme} "Chinese (China)"
   StrCpy ${LangCdeSys} "zh_CN"
   StrCpy ${LangNmeSys} "Chinese"
   StrCpy ${LangSysEnc} "936"
  ${endif}

  ${if} ${LangISOCode} = 3079
   StrCpy ${LangCdeSys} "de"
   StrCpy ${LangNmeSys} "Deutsch"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 3081
   StrCpy ${LangCdeSys} "en"
   StrCpy ${LangNmeSys} "English"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 3082
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 3084
   StrCpy ${LangCdeSys} "fr"
   StrCpy ${LangNmeSys} "Français"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 4103
   StrCpy ${LangCdeSys} "de"
   StrCpy ${LangNmeSys} "Deutsch"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 4105
   StrCpy ${LangCdeSys} "en"
   StrCpy ${LangNmeSys} "English"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 4108
   StrCpy ${LangCdeSys} "fr"
   StrCpy ${LangNmeSys} "Français"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

# currently no dictionary available
  ${if} ${LangISOCode} = 5124
   StrCpy ${LangNme} "Chinese (China)"
   StrCpy ${LangCdeSys} "zh_CN"
   StrCpy ${LangNmeSys} "Chinese"
   StrCpy ${LangSysEnc} "936"
  ${endif}

  ${if} ${LangISOCode} = 5129
   StrCpy ${LangCdeSys} "en"
   StrCpy ${LangNmeSys} "English"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 5132
   StrCpy ${LangCdeSys} "fr"
   StrCpy ${LangNmeSys} "Français"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 7177
   StrCpy ${LangCdeSys} "en"
   StrCpy ${LangNmeSys} "English"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 11274
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 13322
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 14346
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

  ${if} ${LangISOCode} = 20490
   StrCpy ${LangCdeSys} "es"
   StrCpy ${LangNmeSys} "Español"
   StrCpy ${LangSysEnc} "1252"
  ${endif}

!macroend

#------------------------------------------

# To set later LyX's menu language
# therefore here are only declared the languages LyX is translated to

!macro GetLangCode LangCde Name LangEnc

  ${if} ${Name} = "Arabic"
   StrCpy ${LangCde} "ar_SA"
   StrCpy ${LangEnc} "cp-1256"
  ${endif}

  ${if} ${Name} == "Català"
   StrCpy ${LangCde} "ca_ES"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Ceština"
   StrCpy ${LangCde} "cs_CZ"
   StrCpy ${LangEnc} "cp-1250"
  ${endif}

  ${if} ${Name} == "Chinese (China)"
   StrCpy ${LangCde} "zh_CN"
   StrCpy ${LangEnc} "cp-936"
  ${endif}

  ${if} ${Name} == "Chinese (Taiwan)"
   StrCpy ${LangCde} "zh_TW"
   StrCpy ${LangEnc} "cp-936"
  ${endif}

  ${if} ${Name} == "Dansk"
   StrCpy ${LangCde} "da_DK"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Deutsch"
   StrCpy ${LangCde} "de_DE"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "English"
   StrCpy ${LangCde} "en_EN"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Español"
   StrCpy ${LangCde} "es_ES"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Euskara"
   StrCpy ${LangCde} "eu_EU"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Français"
   StrCpy ${LangCde} "fr_FR"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Galego"
   StrCpy ${LangCde} "gl_ES"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Hebrew"
   StrCpy ${LangCde} "he_IL"
   StrCpy ${LangEnc} "cp-1255"
  ${endif}

  ${if} ${Name} == "Italiano"
   StrCpy ${LangCde} "it_IT"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Japanese"
   StrCpy ${LangCde} "ja_JP"
   StrCpy ${LangEnc} "cp-932"
  ${endif}

  ${if} ${Name} == "Korean"
   StrCpy ${LangCde} "ko" # no country code, see the definition in the file "SVN ~/lib/languages"
   StrCpy ${LangEnc} "cp-949"
  ${endif}

  ${if} ${Name} == "Magyar"
   StrCpy ${LangCde} "hu_HU"
   StrCpy ${LangEnc} "cp-1250"
  ${endif}

  ${if} ${Name} == "Nederlands"
   StrCpy ${LangCde} "nl_NL"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Norsk"
   StrCpy ${LangCde} "nb_NO"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Nynorsk"
   StrCpy ${LangCde} "nn_NO"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Polski"
   StrCpy ${LangCde} "pl_PL"
   StrCpy ${LangEnc} "cp-1250"
  ${endif}

  ${if} ${Name} == "Português"
   StrCpy ${LangCde} "pt_PT"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Româna"
   StrCpy ${LangCde} "ro_RO"
   StrCpy ${LangEnc} "cp-1250"
  ${endif}

  ${if} ${Name} == "Russian"
   StrCpy ${LangCde} "ru_RU"
   StrCpy ${LangEnc} "cp-1251"
  ${endif}

  ${if} ${Name} == "Slovenský"
   StrCpy ${LangCde} "sk_SK"
   StrCpy ${LangEnc} "cp-1250"
  ${endif}

  ${if} ${Name} == "Slovenšcina"
   StrCpy ${LangCde} "sl_SI"
   StrCpy ${LangEnc} "cp-1250"
  ${endif}

  ${if} ${Name} == "Suomi"
   StrCpy ${LangCde} "fi_FI"
   StrCpy ${LangEnc} "cp-1252"
  ${endif}

  ${if} ${Name} == "Türkçe"
   StrCpy ${LangCde} "tr_TR"
   StrCpy ${LangEnc} "cp-1254"
  ${endif}

  ${if} ${Name} == "Ukrainian"
   StrCpy ${LangCde} "uk_UA"
   StrCpy ${LangEnc} "cp-1251"
  ${endif}

!macroend

#--------------------
# function that creates the installer page for the language setting
Function SelectMenuLanguage

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "io_ui_language.ini"

  ${if} $LangName == ""
   # translate NSIS's language code to the language name
   !insertmacro TranslateLangCode $LangName $LangNameSys $LangCodeSys $Language $LangSysEncoding
  ${endif}

  ${if} $LangName == ""
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_ui_language.ini" "Field 2" "State" "English"
  ${else}
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_ui_language.ini" "Field 2" "State" "$LangName"
  ${endif}

  !insertmacro MUI_HEADER_TEXT "$(LangSelectHeader)" ""
  !insertmacro MUI_INSTALLOPTIONS_WRITE "io_ui_language.ini" "Field 1" "Text" "$(AvailableLang)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "io_ui_language.ini"
  !insertmacro MUI_INSTALLOPTIONS_READ $LangName "io_ui_language.ini" "Field 2" "State"

FunctionEnd

Function SelectMenuLanguage_LeaveFunction

  !insertmacro MUI_INSTALLOPTIONS_READ $LangName "io_ui_language.ini" "Field 2" "State"
  # Get the language code
  StrCpy $LangCode ""
  !insertmacro GetLangCode $LangCode $LangName $LangEncoding

FunctionEnd

