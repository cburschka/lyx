/*

Checks for Windows version

*/


;--------------------------------
;Macros

!macro CheckPrivileges

  Push $R0

  UserInfo::GetAccountType
  Pop $R0
  
  ${if} $R0 == "Admin"
    StrCpy $AdminOrPowerUser ${TRUE}
  ${elseif} $R0 == "Power"
    StrCpy $AdminOrPowerUser ${TRUE}
  ${else}
    StrCpy $AdminOrPowerUser ${FALSE}
  ${endif}
  
  Pop $R0

!macroend

!macro CommandLineParameter UNINSTALL

  Exch $R0
  Push $R1
  Push $R2
  
  StrLen $R1 $R0
  
  Push $CMDLINE
  Push $R0
  Call ${UNINSTALL}StrStr
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

!macroend

;--------------------------------
;Functions

Function CheckPrivileges
  !insertmacro CheckPrivileges
FunctionEnd

Function un.CheckPrivileges
  !insertmacro CheckPrivileges
FunctionEnd

Function CommandLineParameter
  !insertmacro CommandLineParameter ""
FunctionEnd

Function un.CommandLineParameter
  !insertmacro CommandLineParameter un.
FunctionEnd

Function CheckWindows

  Push $R0
  Push $R1

  ;Check for Windows NT 5.0 or later (2000, XP, 2003 etc.)

  ReadRegStr $R0 HKLM "Software\Microsoft\Windows NT\CurrentVersion" "CurrentVersion"
  ${VersionCompare} $R0 "5.0" $R1
  
  ${if} $R1 == "2"
    MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} ${APP_VERSION} only supports Windows 2000, XP, 2003 and later."
    Quit
  ${endif}
    
  Pop $R1
  Pop $R0

FunctionEnd

Function UpdatePathEnvironment

  Push $R0
  Push $R1
  
  ;Updates the path environment variable of the instaler process to the latest system value
  
  ReadRegStr $R0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" Path
  ReadRegStr $R1 HKCU "Environment" Path
  
  System::Call 'kernel32::SetEnvironmentVariableA(t, t) i("Path", "$R0;$R1").r0'
  
  Pop $R1
  Pop $R0

FunctionEnd

Function InitInstaller

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
    
    ${endif}
    
  ${endif}
  
  ${if} ${silent}
    Call InitUser
  ${endif}
  
  Pop $R0

FunctionEnd

Function un.InitUnInstaller

  ;Set the correct shell context depending on whether LyX has been installed
  ;for the current user or all users

  Push "/CurrentUser"
  Call un.CommandLineParameter
  Pop $CurrentUserInstall

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

