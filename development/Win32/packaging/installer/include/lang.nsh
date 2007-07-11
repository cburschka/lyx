/*

Translations for installer and LyX itself

*/

!macro LanguageList COMMAND

  #List of LyX translations with ISO code and language identifier

  ${COMMAND} "Bokmål" nb_NO 1044
  # ${COMMAND} "Català" ca_ES 1027
  ${COMMAND} "Ceština" cs_CZ 1029
  # ${COMMAND} "Chinese (Simplified)" zh_CN 2052
  ${COMMAND} "Chinese (Traditional)" zh_TW 1028
  # ${COMMAND} "Dansk" da_DK 1030
  ${COMMAND} "Deutsch" de_DE 1031
  ${COMMAND} "English" en_EN 1033
  ${COMMAND} "Español" es_ES 1034
  ${COMMAND} "Euskara" eu_EU 1069
  ${COMMAND} "Français" fr_FR 1036
  ${COMMAND} "Galego" gl_ES 1036
  # ${COMMAND} "Hebrew" he_IL 1037
  ${COMMAND} "Italiano" it_IT 1057
  ${COMMAND} "Japanese" ja_JP 1041
  ${COMMAND} "Korean" ko 1042
  ${COMMAND} "Magyar" hu_HU 1038
  # ${COMMAND} "Nederlands" nl_NL 1043
  ${COMMAND} "Nynorsk" nn_NO 2068
  # ${COMMAND} "Português" pt_PT 1046
  ${COMMAND} "Polski"  pl_PL 1045
  ${COMMAND} "Româna" ro_RO 1048
  # ${COMMAND} "Russian" ru_RU 1049
  # ${COMMAND} "Slovencina" sk_SK 1060
  # ${COMMAND} "Slovenšcina" sl_SI 1051
  # ${COMMAND} "Suomi" fi_FI 1035
  # ${COMMAND} "Svenska" sv_SE 1053
  ${COMMAND} "Türkçe" tr_TR 1055
  # ${COMMAND} "Walon" wa_BE 1055

!macroend

!macro LanguageString name text

  !ifndef "${name}"
    !define "${name}" "${text}"
  !endif
  
!macroend

!macro LanguageStringAdd lang_name name

  #Takes a define and puts that into a language string
  LangString "${name}" "${LANG_${lang_name}}" "${${name}}"
  !undef "${name}"

!macroend

!macro LanguageStringCreateExternal component

  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_TITLE
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_SUBTITLE
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_INFO_${SETUPTYPE_NAME}
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_${SETUPTYPE_NAME}
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_FOLDER
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_FOLDER_INFO
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_NONE
  ${LanguageStringAdd} TEXT_EXTERNAL_${component}_NOTFOUND
  
!macroend

!macro LanguageStringCreate lang_name

  #Creates all language strings
  !insertmacro ReDef LanguageStringAdd '!insertmacro LanguageStringAdd "${lang_name}"'

  ${LanguageStringAdd} TEXT_NO_PRIVILEDGES
  
  ${LanguageStringAdd} TEXT_INSTALL_CURRENTUSER

  ${LanguageStringAdd} TEXT_WELCOME_${SETUPTYPE_NAME}

  ${LanguageStringAdd} TEXT_USER_TITLE
  ${LanguageStringAdd} TEXT_USER_SUBTITLE
  ${LanguageStringAdd} TEXT_USER_INFO  
  ${LanguageStringAdd} TEXT_USER_CURRENT
  ${LanguageStringAdd} TEXT_USER_ALL
  
  ${LanguageStringAdd} TEXT_REINSTALL_TITLE
  ${LanguageStringAdd} TEXT_REINSTALL_SUBTITLE
  ${LanguageStringAdd} TEXT_REINSTALL_INFO
  ${LanguageStringAdd} TEXT_REINSTALL_ENABLE
  
  ${LanguageStringAdd} TEXT_EXTERNAL_NOPRIVILEDGES

  !insertmacro LanguageStringCreateExternal LaTeX
  !insertmacro LanguageStringCreateExternal ImageMagick
  !insertmacro LanguageStringCreateExternal Ghostscript

  ${LanguageStringAdd} TEXT_VIEWER_TITLE
  ${LanguageStringAdd} TEXT_VIEWER_SUBTITLE
  ${LanguageStringAdd} TEXT_VIEWER_INFO_${SETUPTYPE_NAME}
  ${LanguageStringAdd} TEXT_VIEWER_${SETUPTYPE_NAME}
  
  ${LanguageStringAdd} TEXT_DICT_TITLE
  ${LanguageStringAdd} TEXT_DICT_SUBTITLE
  ${LanguageStringAdd} TEXT_DICT_TOP
  ${LanguageStringAdd} TEXT_DICT_LIST
  
  ${LanguageStringAdd} TEXT_LANGUAGE_TITLE
  ${LanguageStringAdd} TEXT_LANGUAGE_SUBTITLE
  ${LanguageStringAdd} TEXT_LANGUAGE_INFO 

  !ifndef SETUPTYPE_BUNDLE
    ${LanguageStringAdd} TEXT_DOWNLOAD_FAILED_LATEX
    ${LanguageStringAdd} TEXT_DOWNLOAD_FAILED_IMAGEMAGICK
    ${LanguageStringAdd} TEXT_DOWNLOAD_FAILED_GHOSTSCRIPT
    ${LanguageStringAdd} TEXT_DOWNLOAD_FAILED_VIEWER
  !endif 
  ${LanguageStringAdd} TEXT_DOWNLOAD_FAILED_DICT
  
  ${LanguageStringAdd} TEXT_NOTINSTALLED_LATEX
  ${LanguageStringAdd} TEXT_NOTINSTALLED_IMAGEMAGICK
  ${LanguageStringAdd} TEXT_NOTINSTALLED_GHOSTSCRIPT
  ${LanguageStringAdd} TEXT_NOTINSTALLED_VIEWER
  ${LanguageStringAdd} TEXT_NOTINSTALLED_DICT
  
  ${LanguageStringAdd} TEXT_FINISH_DESKTOP
  ${LanguageStringAdd} TEXT_FINISH_WEBSITE
  
  ${LanguageStringAdd} UNTEXT_WELCOME

!macroend

!macro IncludeLang langname

  !ifndef DICT_NAME
    !define DICT_NAME $R1
  !endif

  #Include both NSIS/LyX language file
  
  !insertmacro MUI_LANGUAGE "${langname}"
  
  !include "lang\${langname}.nsh"
  !include "lang\English.nsh" ;Use English for missing strings in translation
  
  !insertmacro LanguageStringCreate "${langname}"
  
!macroend
