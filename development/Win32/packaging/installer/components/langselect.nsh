/*

Selection of language for LyX interface

*/

#--------------------------------
#Macros

!macro SetSystemLangItem VAR LANGNAME LANGISOCODE LANGID

  ${if} ${VAR} == ${LANGID}
    !insertmacro MUI_INSTALLOPTIONS_WRITE "langselect.ini" "Field 2" "State" "${LANGNAME}"
  ${endif}

!macroend

!macro SetSystemLang VAR

  !insertmacro LanguageList '!insertmacro SetSystemLangItem ${VAR}'
  
!macroend

!macro GetISOLangCodeItem VAR_LANGNAME VAR_LANGISOCODE LANGNAME LANGISOCODE LANGID

  ${if} ${VAR_LANGNAME} == "${LANGNAME}"
    StrCpy ${VAR_LANGISOCODE} "${LANGISOCODE}"
  ${endif}
  
!macroend

!macro GetISOLangCode VAR_LANGNAME VAR_LANGISOCODE

  #Get the language code for the selected language name

  !insertmacro LanguageList '!insertmacro GetISOLangCodeItem ${VAR_LANGNAME} ${VAR_LANGISOCODE}'

!macroend

#--------------------------------
#Functions

Function InitLanguage

  #Set the default for the language selection dialog to the Windows language

  System::Call "kernel32::GetUserDefaultUILanguage()i.R0"
  !insertmacro SetSystemLang $R0
  
FunctionEnd

#--------------------------------
#Page functions

Function PageLanguage

  !insertmacro MUI_HEADER_TEXT "$(TEXT_LANGUAGE_TITLE)" "$(TEXT_LANGUAGE_SUBTITLE)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "langselect.ini"
 
FunctionEnd

Function PageLanguageValidate

  !insertmacro MUI_INSTALLOPTIONS_READ $LangName "langselect.ini" "Field 2" "State"
  !insertmacro GetISOLangCode $LangName $LangISOCode
  
FunctionEnd