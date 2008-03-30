/*

langselect.nsh

Selection of language for LyX interface

*/

Var LangSelectPage
Var LangSelectPage.Info
Var LangSelectPage.DropList

Var LangSystem

!macro LoadLang VAR LANGNAME LANGISOCODE LANGID

  ${NSD_CB_AddString} $LangSelectPage.DropList "${LANGNAME}" 

!macroend

!macro SetSystemLang VAR LANGNAME LANGISOCODE LANGID

  ${If} ${VAR} == ${LANGID}
    ${NSD_CB_SelectString} $LangSelectPage.DropList "${LANGNAME}" 
  ${EndIf}

!macroend

!macro GetISOLangCode VAR_LANGNAME VAR_LANGISOCODE LANGNAME LANGISOCODE LANGID

  ${If} ${VAR_LANGNAME} == "${LANGNAME}"
    StrCpy ${VAR_LANGISOCODE} "${LANGISOCODE}"
  ${EndIf}
  
!macroend

# Page functions

Function PageLanguage

  # Build the dialog

  !insertmacro MUI_HEADER_TEXT "$(TEXT_LANGUAGE_TITLE)" "$(TEXT_LANGUAGE_SUBTITLE)"
  
  nsDialogs::Create /NOUNLOAD 1018
  Pop $LangSelectPage
  
  ${NSD_CreateLabel} 0u 0u 300u 20u $(TEXT_LANGUAGE_INFO)
  Pop $LangSelectPage.Info
  
  ${NSD_CreateDropList} 0u 40u 110u 100u ""
  Pop $LangSelectPage.DropList
  
  # Load list of languages
  !insertmacro Languages '!insertmacro LoadLang $LangSystem'
  
  ${If} $LangName == ""
    # Set the default for the language selection dialog to the Windows language
    System::Call "kernel32::GetUserDefaultUILanguage()i.s"
    Pop $LangSystem
    !insertmacro Languages '!insertmacro SetSystemLang $LangSystem'
  ${Else}
    # Restore previous user selection
    ${NSD_CB_SelectString} $LangSelectPage.DropList $LangName    
  ${EndIf}
  
  nsDialogs::Show
 
FunctionEnd

Function PageLanguageValidate

  ${NSD_GetText} $LangSelectPage.DropList $LangName
  # Convert to ISO code
  !insertmacro Languages '!insertmacro GetISOLangCode $LangName $LangISOCode'

FunctionEnd
