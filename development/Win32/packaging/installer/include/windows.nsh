/*

Checks for Windows version

*/

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

Function CheckPriviledges

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
  
  ${if} $AdminOrPowerUser != ${TRUE}
    MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_NO_PRIVILEDGES)
  ${endif}
  
  Pop $R0
  
FunctionEnd

Function un.SetShellContext

  Push $R0

  ;Set the correct shell context depending on whether LyX has been installed for the current user or all users

  UserInfo::GetAccountType
  Pop $R0
  
  ${if} $R0 == "Admin"
    StrCpy $AdminOrPowerUser ${TRUE}
  ${endif}
  
  ${if} $R0 == "Power"
    StrCpy $AdminOrPowerUser ${TRUE}
  ${endif}
  
  ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}" "UninstallString"
  
  ${if} $R0 != ""
  
    ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}" "UninstallString"

    ${if} $R0 != ""
    
      ${if} $AdminOrPowerUser == ${FALSE}
        MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} has been installed for all users. Therefore you need Administrator or Power User Priviledges to uninstall."
        Quit
      ${else}
        SetShellVarContext all
      ${endif}
      
    ${endif}
    
  ${else}
  
    SetShellVarContext current
  
  ${endif}
  
  Pop $R0
  
FunctionEnd
