/*

Selection of language for LyX interface

*/

;--------------------------------
;Macros

!macro SystemLanguage lang_name lang_code

  ${if} $R0 == ${lang_code}
    !insertmacro MUI_INSTALLOPTIONS_WRITE "langselect.ini" "Field 2" "State" "${lang_name}"
  ${endif}

!macroend

!macro GetLyXLangCodeItem var_name var_langcode name langcode

  ${if} ${var_name} == "${name}"
    StrCpy ${var_langcode} "${langcode}"
  ${endif}
  
!macroend

!macro GetLyXLangCode var_name var_langcode
 
  !define LANGCODE '!insertmacro GetLyXLangCodeItem ${var_name} ${var_langcode}'
  
  ${LANGCODE} "Basque" "eu_EU"  
  ${LANGCODE} "Dansk" "da_DK"
  ${LANGCODE} "Dutch" "nl_NL"
  ${LANGCODE} "English" "en_EN"
  ${LANGCODE} "Finnish" "fi_FI"
  ${LANGCODE} "French" "fr_FR"    
  ${LANGCODE} "German" "de_DE"
  ${LANGCODE} "Hungarian" "hu_HU"
  ${LANGCODE} "Italian" "it_IT"
  ${LANGCODE} "Norwegian" "no_NO"
  ${LANGCODE} "Norwegian (Nynorsk)" "no_NY"
  ${LANGCODE} "Polish" "pl_PL"
  ${LANGCODE} "Romanish" "ro_RO"
  ${LANGCODE} "Russian" "ru_RU"
  ${LANGCODE} "Slovak" "sk_SK"
  ${LANGCODE} "Slovian" "sl_SI"
  ${LANGCODE} "Spanish" "es_ES"
  ${LANGCODE} "Turkish" "tr_TR"

!macroend

;--------------------------------
;Functions

Function InitLanguage

  ;Set the default for the LyX language selection dialog

  System::Call "kernel32::GetUserDefaultUILanguage()i.R0"
  
  !define SYSTEM_LANG '!insertmacro SystemLanguage'
  
  ${SYSTEM_LANG} "Basque" 1069
  ${SYSTEM_LANG} "Danish" 1030
  ${SYSTEM_LANG} "Dutch" 1043
  ${SYSTEM_LANG} "English" 1033
  ${SYSTEM_LANG} "Finnish" 1035
  ${SYSTEM_LANG} "French" 1036
  ${SYSTEM_LANG} "German" 1031
  ${SYSTEM_LANG} "Hungarian" 1038
  ${SYSTEM_LANG} "Italian" 1057
  ${SYSTEM_LANG} "Norwegian" 1044
  ${SYSTEM_LANG} "Norwegian (Nynorsk)" 2068
  ${SYSTEM_LANG} "Polish" 1045
  ${SYSTEM_LANG} "Romanian" 1048
  ${SYSTEM_LANG} "Russian" 1049
  ${SYSTEM_LANG} "Slovak" 1051
  ${SYSTEM_LANG} "Slovenian" 1060
  ${SYSTEM_LANG} "Spanish" 1034
  ${SYSTEM_LANG} "Turkish" 1055
  
FunctionEnd

;--------------------------------
;Page functions

Function PageLanguage

  !insertmacro MUI_HEADER_TEXT "$(TEXT_LANGUAGE_TITLE)" "$(TEXT_LANGUAGE_SUBTITLE)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "langselect.ini"
 
FunctionEnd

Function PageLanguageValidate

  !insertmacro MUI_INSTALLOPTIONS_READ $LangName "langselect.ini" "Field 2" "State"
  !insertmacro GetLyXLangCode $LangName $LangCode
  
FunctionEnd