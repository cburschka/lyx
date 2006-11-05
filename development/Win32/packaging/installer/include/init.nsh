/*

Installer and uninstaller initialization

*/

;--------------------------------
;Functions

Function CommandLineParameter

  Exch $R0
  Push $R1
  Push $R2
  
  StrLen $R1 $R0
  
  Push $CMDLINE
  Push $R0
  Call StrStr
  Pop $R2
  
  StrCpy $R2 $R2 $R1
  
  ${if} $R2 == $R0
    StrCpy $R0 ${TRUE}
  ${else}
    StrCpy $R0 ${FALSE} 
  ${endif}
  
  Push $R2
  Push $R1
  Exch $R0

FunctionEnd

Function .onInit

  ${unless} ${silent}
    Banner::show /NOUNLOAD "Checking system"
  ${endif}
  
  ;Check all dependencies
  
  Call CheckWindows
  Call CheckPrivileges
  Call LoadInstaller
  Call SearchAll

  ${unless} ${silent}
    Banner::destroy
  ${endif}

FunctionEnd

Function un.onInit

  Call un.CheckPrivileges
  Call un.LoadUnInstaller
  
FunctionEnd

Function LoadInstaller

  ;Set the correct shell context depending on command line parameter
  ;and priviledges

  Push $R0

  Push "/CurrentUser"
  Call CommandLineParameter
  Pop $CurrentUserInstall

  ${if} $CurrentUserInstall == ${TRUE}
  
    SetShellVarContext current
  
  ${else}
  
    ${if} $AdminOrPowerUser == ${TRUE}
      
      SetShellVarContext all
      
    ${else}
    
      ;Display an error when the /AllUsers command line parameter is used
      ;by a user without Administrator or Power User priviledges
       
      Push "/AllUsers"
      Call CommandLineParameter
      Pop $R0
  
      ${if} $R0 == ${TRUE}
        MessageBox MB_OK|MB_ICONSTOP "You need Administrator or Power User privileges to install ${APP_NAME} for all users."
        Quit
      ${endif}
      
      SetShellVarContext current
      StrCpy $CurrentUserInstall ${TRUE}
    
    ${endif}
    
  ${endif}
  
  ${if} ${silent}
    Call InitUser
  ${endif}
  
  Pop $R0

FunctionEnd

Function un.LoadUnInstaller

  ;Set the correct shell context depending on whether LyX has been installed
  ;for the current user or all users

  ReadRegStr $R0 HKCU ${APP_REGKEY} ""
  
  ${if} $R0 == $INSTDIR
    StrCpy $CurrentUserInstall ${TRUE}
  ${endif}

  ${if} $CurrentUserInstall == ${TRUE}
  
    SetShellVarContext current
  
  ${else}
  
    ${if} $AdminOrPowerUser == ${FALSE}
      MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} has been installed for all users. Therefore you need Administrator or Power User privileges to uninstall."
      Quit
    ${else}
      SetShellVarContext all
    ${endif}
  
  ${endif}

FunctionEnd
