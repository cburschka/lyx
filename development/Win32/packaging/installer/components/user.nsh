/*

Install type setting (current user/all users)

*/

#--------------------------------
#Macros

# COMPONENT can be LaTeX ImageMagick and Ghostscript
!macro GetDirExternal COMPONENT

  # APP_REGKEY_SETUP = "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
  # where ${APP_NAME}${APP_SERIES_KEY} is something like LyX15
  ReadRegStr $R0 SHELL_CONTEXT "${APP_REGKEY_SETUP}" "${COMPONENT} Path"
  
  # BIN_LATEX etc are defined in settings.nsh
  ${if} ${FileExists} "$R0\${BIN_${COMPONENT}}"

    ${if} $R0 != ""
      # define variables like PathLATEX
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
  
  # Set directories in dialogs
  # Macro defined in include/gui.sh, parameters are COMPONENT CURRENTUSER_POSSIBLE
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
  
  # shell var context is important because it determines the actual
  # meaning of variables like $DESKTOP
  ${if} $R0 == "1"
    SetShellVarContext all
    StrCpy $CurrentUserInstall ${FALSE}
  ${else}
    SetShellVarContext current
    StrCpy $CurrentUserInstall ${TRUE}
  ${endif}
  
  Call InitUser
  
FunctionEnd
