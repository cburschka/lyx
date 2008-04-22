/*

init.nsh

Initialization function

*/

#--------------------------------
# Installer initialization

!macro PRINTER_INIT

  ${If} ${AtLeastWinVista}
    StrCpy $PrinterConf "printui.exe"
  ${Else}
    StrCpy $PrinterConf "rundll32.exe printui.dll,PrintUIEntry"
  ${EndIf}

!macroend

Function .onInit

  ${IfNot} ${IsNT}
  ${OrIfNot} ${AtLeastWin2000}
    MessageBox MB_OK|MB_ICONSTOP "${APP_NAME} ${APP_VERSION} requires Windows 2000 or later."
    Quit
  ${EndIf}

  !insertmacro PRINTER_INIT
  !insertmacro MULTIUSER_INIT
 
  ${IfNot} ${Silent}
    # Warn the user when no Administrator or Power user privileges are available
    # These privileges are required to install ImageMagick or Ghostscript
    ${If} $MultiUser.Privileges != "Admin"
    ${andif} $MultiUser.Privileges != "Power"
      MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_NO_PRIVILEDGES)
    ${EndIf}
    
    # Show banner while installer is intializating 
    Banner::show /NOUNLOAD "Checking system"
  ${EndIf}
 
  Call SearchExternal
  Call InitExternal

  ${IfNot} ${Silent}
    Banner::destroy
  ${EndIf}

FunctionEnd

Function un.onInit

  !insertmacro PRINTER_INIT
  !insertmacro MULTIUSER_UNINIT

FunctionEnd

#--------------------------------
# User initialization

Var ComponentPath
Var LyXLangName

# COMPONENT can be LaTeX ImageMagick and Ghostscript
!macro EXTERNAL_INIT COMPONENT

  # APP_REGKEY_SETUP = "Software\${APP_NAME}${APP_SERIES_KEY}\Setup"
  # where ${APP_NAME}${APP_SERIES_KEY} is something like LyX16
  ReadRegStr $ComponentPath SHELL_CONTEXT "${APP_REGKEY_SETUP}" "${COMPONENT} Path"
  
  # BIN_LATEX etc are defined in settings.nsh
  ${If} ${FileExists} "$ComponentPath\${BIN_${COMPONENT}}"
    # set variables like PathLaTeX
    StrCpy $Path${COMPONENT} $ComponentPath
  ${EndIf}

!macroend

Function InitUser

  # Get directories of components from registry
  
  !insertmacro EXTERNAL_INIT LaTeX
  !insertmacro EXTERNAL_INIT ImageMagick
  !insertmacro EXTERNAL_INIT Ghostscript
  
  # Get LyX language
  
  ReadRegStr $LyXLangName SHELL_CONTEXT "${APP_REGKEY_SETUP}" "LyX Language"
  
  ${If} $LyXLangName != ""
    StrCpy $LangName $LyXLangName
  ${EndIf}
  
FunctionEnd
