/*

Language file handling

*/

!macro LanguageString name text

  !insertmacro ReDef "${name}" "${text}"
  
!macroend

!macro LanguageStringAdd lang_name name

  ;Takes a define and puts that into a language string
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

  ;Creates all language strings
  !insertmacro ReDef LanguageStringAdd '!insertmacro LanguageStringAdd "${lang_name}"'

  ${LanguageStringAdd} TEXT_NO_PRIVILEDGES

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

  ;Include both NSIS/LyX language file
  
  !insertmacro MUI_LANGUAGE "${langname}"
  
  !include "lang\${langname}.nsh"
  !include "lang\English.nsh" ;Use English for missing strings in translation
  
  !insertmacro LanguageStringCreate "${langname}"
  
!macroend
