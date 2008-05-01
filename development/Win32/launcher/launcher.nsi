/*

LyX for Windows Launcher
Author: Joost Verburg

This will be installed as lyx.exe.

The application will setup the environment variables based on registry
settings and obtain the command line output of lyxc.exe, which can be shown
while debugging or in case of a crash. Version information and an icon are
also included.

*/

!include MUI.nsh
!include LogicLib.nsh
!include FileFunc.nsh
!include StrFunc.nsh
!insertmacro GetParameters
!insertmacro GetParent
${StrStr}

# Configuration from installer
!include "..\packaging\installer\settings.nsh"
!include "..\packaging\installer\include\declarations.nsh"

#--------------------------------
# Settings

Caption "${APP_NAME} ${APP_VERSION}"
OutFile LyXLauncher.exe
BrandingText " "

#--------------------------------
# Windows Vista settings

RequestExecutionLevel user

#--------------------------------
# Variables

Var Parameters
Var Debug
Var LyXLanguage
Var ReturnValue

Var ResultText
Var ResultSubText

Var LyXFolder

Var LyXSetting
Var LyXSettingValue

Var EnvironmentVariable
Var EnvironmentVariableValue

#--------------------------------
# User interface for debug output

!define MUI_ICON "..\packaging\icons\lyx.ico"
!define MUI_CUSTOMFUNCTION_GUIINIT InitInterface

!define MUI_INSTFILESPAGE_FINISHHEADER_TEXT $ResultText
!define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT $ResultSubText
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_LANGUAGE English

ShowInstDetails show

#--------------------------------
# Windows API constants

!define SWP_NOSIZE 0x1
!define MONITOR_DEFAULTTONEAREST 0x2

!define SM_CYCAPTION 4
!define SM_CXSIZEFRAME 32
!define SM_CYSIZEFRAME 33

#--------------------------------
# Version information

VIProductVersion "${APP_VERSION_NUMBER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APP_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APP_INFO}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APP_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${APP_COPYRIGHT}"

#--------------------------------
# Macros

!macro SystemCall STACK

  Push "${STACK}"
  CallInstDLL "$EXEDIR\System.dll" Call

!macroend

!macro GetLyXSetting NAME VAR

  Push "${NAME}"
  Call GetLyXSetting
  Pop "${VAR}"

!macroend

#--------------------------------
# Main application

Section -Prepare

  ${If} $Debug == ${FALSE}
    HideWindow
  ${EndIf}
  
  # Hide controls we don't need
  FindWindow $R0 "#32770" "" $HWNDPARENT
  GetDlgItem $R0 $R0 1004
  ShowWindow $R0 ${SW_HIDE}
  
  # Get LyX folder
  ${GetParent} $EXEDIR $LyXFolder  
  
  # Debug info
  !insertmacro MUI_HEADER_TEXT "Debugging LyX" "The events you have chosen \
      are being logged."
  SetDetailsPrint textonly
  DetailPrint "Debug log:"
  SetDetailsPrint listonly
  
  # LyX Language
  !insertmacro GetLyXSetting "Language" $LyXLanguage
  
  # Set language for gettext
  ${if} $LyXLanguage != ""
    Push LC_ALL
    Push $LyXLanguage
    Call SetEnvironmentVariable
  ${endif}
  
  # Apparently the output charset needs to be set to some value,
  # otherwise no non-ASCII characters will be displayed
  Push OUTPUT_CHARSET
  Push -
  Call SetEnvironmentVariable
  
  # Location of Aiksaurus data
  Push AIK_DATA_DIR
  Push "$LyXFolder\aiksaurus"
  Call SetEnvironmentVariable

  # Location of Ghostscript
  Push LYX_GHOSTSCRIPT_EXE
  Push "$LyXFolder\ghostscript\bin\gswin32c.exe"
  Call SetEnvironmentVariable
  Push LYX_GHOSTSCRIPT_DLL
  Push "$LyXFolder\ghostscript\bin\gsdll32.dll"
  Call SetEnvironmentVariable
  Push LYX_GHOSTSCRIPT_FONTS
  Push "$LyXFolder\ghostscript\fonts"
  Call SetEnvironmentVariable
  
  # Ghostscript resources
  Push GS_DLL
  Push "$LyXFolder\ghostscript\bin\gsdll32.dll"
  Call SetEnvironmentVariable  
  Push GS_LIB
  Push "$LyXFolder\ghostscript\lib;$LyXFolder\ghostscript\fonts;$LyXFolder\ghostscript\Resource"
  Call SetEnvironmentVariable

SectionEnd

Section -Launch
  
  # Start LyX and capture the command line output
  
  Push '"$EXEDIR\lyx.exe" $Parameters'
  CallInstDLL "$EXEDIR\Console.dll" ExecToLog
  Pop $ReturnValue #Return value
  
SectionEnd

Section -Debug
  
  ${If} $Debug == ${FALSE}
  
    # Check whether something went wrong
    
    ${If} $ReturnValue == "error"
  
      # Probably the file does not exist
      MessageBox MB_OK|MB_ICONSTOP "Failed to start LyX."
    
    ${ElseIf} $ReturnValue != 0
    
      # LyX has crashed
      MessageBox MB_YESNO|MB_ICONSTOP \
          "LyX has been closed because of an unexpected situation.$\n\
          This is most likely caused by a flaw in the software.$\n$\n\
          When you open your documents again, you will be able$\n\
          to restore an emergency save and continue working.$\n$\n\
          Would you like to view detailed information about this error?" \
          IDYES debug IDNO no_debug
  
    ${EndIf}
    
    no_debug:
    
      Quit
    
    debug:
    
      ShowWindow $R0 ${SW_HIDE}
    
  ${EndIf}
  
  ${If} $ReturnValue != 0
  
    StrCpy $ResultText "Error Information"
    StrCpy $ResultSubText "See Chapter 3 of the LyX Introduction \
        (Help > Introduction) for information about reporting this issue."
   
   ${Else}
   
    StrCpy $ResultText "Debugging Completed"
    StrCpy $ResultSubText "The events you have chosen are logged below."
   
   ${EndIf}
  
  ${If} $Debug == ${FALSE}

    # Put the log window on the screen again
    Push "user32::SetWindowPos(i $HWNDPARENT, i 0, i 133, i 100, i 0, i 0, i ${SWP_NOSIZE})"
    CallInstDLL "$EXEDIR\System.dll" Call
    BringToFront

  ${EndIf}

SectionEnd


#--------------------------------
#Functions

Function InitInterface
  
  #Command line parameters
  Call GetParameters
  Pop $Parameters
  
  #Check for debug mode
  ${StrStr} $R0 $Parameters "-dbg"
  
  ${If} $R0 == ""
    StrCpy $Debug ${FALSE}
  ${Else}
    StrCpy $Debug ${TRUE}
  ${Endif}
  
  ${If} $Debug == ${FALSE}

    # Keep the log window outside the screen to ensure that there will be no flickering
    Push "user32::SetWindowPos(i $HWNDPARENT, i 0, i -32000, i -32000, i 0, i 0, i ${SWP_NOSIZE})"
    CallInstDLL "$EXEDIR\System.dll" Call
  
  ${EndIf}

FunctionEnd

Function GetLyXSetting

  Pop $LyxSetting

  # Get a LyX setting from the registry
  # First try a current user setting, then a system setting

  ReadRegStr $LyXSettingValue HKCU ${APP_REGKEY_SETTINGS} $LyXSetting
  
  ${If} $LyXSettingValue == ""
    ReadRegStr $LyXSettingValue HKLM ${APP_REGKEY_SETTINGS} $LyXSetting
  ${EndIf}
  
  Push $LyXSettingValue

FunctionEnd

Function SetEnvironmentVariable

  # Sets the value of an environment variable
  # Input on stack: name of variable, value

  Pop $EnvironmentVariableValue
  Pop $EnvironmentVariable  
  
  Push 'kernel32::SetEnvironmentVariable(t, t) i("$EnvironmentVariable", "$EnvironmentVariableValue")'
  CallInstDLL "$EXEDIR\System.dll" Call

FunctionEnd
