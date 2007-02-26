/*

Checks for Windows version

*/

#--------------------------------
#Macros

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

#--------------------------------
#Functions

Function CheckPrivileges
  !insertmacro CheckPrivileges
FunctionEnd

Function un.CheckPrivileges
  !insertmacro CheckPrivileges
FunctionEnd

Function CheckWindows

  #Check for Windows NT 5.0 or later (2000, XP, 2003 etc.)

  ${unless} ${IsNT}
    ${andunless} ${AtLeastWin2000}

    MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} ${APP_VERSION} only supports Windows 2000, XP, 2003 and later."

  ${endif}

FunctionEnd

Function UpdatePathEnvironment

  Push $R0
  Push $R1
  
  #Updates the path environment variable of the instaler process to the latest system value
  
  ReadRegStr $R0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" Path
  ReadRegStr $R1 HKCU "Environment" Path
  
  System::Call 'kernel32::SetEnvironmentVariableA(t, t) i("Path", "$R0;$R1").r0'
  
  Pop $R1
  Pop $R0

FunctionEnd
