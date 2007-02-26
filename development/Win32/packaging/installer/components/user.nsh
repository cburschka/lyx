/*

Install type setting (current user/all users)

*/

#--------------------------------
#Macros

!macro GetDirExternal COMPONENT

  ReadRegStr $R0 SHELL_CONTEXT "${APP_REGKEY_SETUP}" "${COMPONENT} Path"
  
  ${if} ${FileExists} "$R0\${BIN_${COMPONENT}}"

    ${if} $R0 != ""
      StrCpy $Path${COMPONENT} $R0
    ${endif}
  
  ${endif}

!macroend

#--------------------------------
#Functions

Function InitUser

  #Get directories from registry

  ReadRegStr $R0 SHELL_CONTEXT "${APP_REGKEY}" ""
  
  ${if} $R0 != ""
    StrCpy $INSTDIR $R0
  ${endif}

  !insertmacro GetDirExternal LaTeX
  !insertmacro GetDirExternal ImageMagick
  !insertmacro GetDirExternal Ghostscript
  
  #Set directories in dialogs

  !insertmacro InitDialogExternalDir latex ${TRUE}
  !insertmacro InitDialogExternalDir imagemagick ${FALSE}
  !insertmacro InitDialogExternalDir ghostscript ${FALSE}
  
  #Get LyX language
  
  ReadRegStr $R0 SHELL_CONTEXT "${APP_REGKEY_SETUP}" "LyX Language"
  
  ${if} $R0 != ""
    !insertmacro MUI_INSTALLOPTIONS_WRITE "langselect.ini" "Field 2" "State" $R0
  ${endif}
  
FunctionEnd

#--------------------------------
#Page functions

Function PageUser

  #Only show page if installing for all users is possible
  ${if} $AdminOrPowerUser == ${FALSE}
    Call InitUser
    Abort
  ${endif}
  
  !insertmacro MUI_HEADER_TEXT $(TEXT_USER_TITLE) $(TEXT_USER_SUBTITLE)
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "user.ini"

FunctionEnd

Function PageUserValidate
  
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "user.ini" "Field 2" "State"
  
  ${if} $R0 == "1"
    SetShellVarContext all
    StrCpy $CurrentUserInstall ${FALSE}
  ${else}
    SetShellVarContext current
    StrCpy $CurrentUserInstall ${TRUE}
  ${endif}
  
  Call InitUser
  
FunctionEnd