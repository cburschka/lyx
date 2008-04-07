/*

LyX for Windows Launcher
Author: Joost Verburg

This will be installed as lyx.exe.

The application will setup the environment variables based on registry
settings and obtain the command line output of lyxc.exe, which can be shown
while debugging or in case of a crash. Version information and an icon are
also included.

*/

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"
!insertmacro GetParameters
${StrStr}

!include "..\packaging\installer\settings.nsh" #Version info from installer

#--------------------------------
#Settings

Caption "${APP_NAME} ${APP_VERSION}"
OutFile lyx.exe
BrandingText " "

#--------------------------------
#Windows Vista settings

RequestExecutionLevel user

#--------------------------------
#Variables

Var Parameters
Var Debug
Var LyXLanguage
Var ReturnValue
Var ResultText
Var ResultSubText

#--------------------------------
#User interface for debug output

!define MUI_ICON "..\packaging\icons\lyx.ico"
!define MUI_CUSTOMFUNCTION_GUIINIT InitInterface

!define MUI_INSTFILESPAGE_FINISHHEADER_TEXT $ResultText
!define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT $ResultSubText
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_LANGUAGE English

ShowInstDetails show

#--------------------------------
#Windows API constants

!define SWP_NOSIZE 0x1
!define MONITOR_DEFAULTTONEAREST 0x2

!define SM_CYCAPTION 4
!define SM_CXSIZEFRAME 32
!define SM_CYSIZEFRAME 33

#--------------------------------
#Version information

VIProductVersion "${APP_VERSION_NUMBER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APP_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APP_INFO}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APP_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${APP_COPYRIGHT}"

#--------------------------------
#Macros

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
#Main application

Section -Prepare

  ${if} $Debug == ${FALSE}
    HideWindow
  ${endif}
  
  #Hide controls we don't need
  FindWindow $R0 "#32770" "" $HWNDPARENT
  GetDlgItem $R0 $R0 1004
  ShowWindow $R0 ${SW_HIDE}  
  
  #Debug info
  !insertmacro MUI_HEADER_TEXT "Debugging LyX" "The events you have chosen \
      are being logged."
  SetDetailsPrint textonly
  DetailPrint "Debug log:"
  SetDetailsPrint listonly
  
  #LyX Language
  !insertmacro GetLyXSetting "Language" $LyXLanguage
  
  #Set language for gettext
  ${if} $LyXLanguage != ""
    Push LC_ALL
    Push $LyXLanguage
    Call SetEnvironmentVariable
  ${endif}
  
  #Apparently the output charset needs to be set to some value,
  #otherwise no non-ASCII characters will be displayed
  Push OUTPUT_CHARSET
  Push -
  Call SetEnvironmentVariable
  
  #Point to the Aiksaurus data in the LyX folder
  Push AIK_DATA_DIR
  Push "$EXEDIR\..\aiksaurus"
  Call SetEnvironmentVariable

SectionEnd

Section -Launch
  
  #Start LyX and capture the command line output
  
  Push '"$EXEDIR\lyxc.exe" $Parameters'
  CallInstDLL "$EXEDIR\Console.dll" ExecToLog
  Pop $ReturnValue #Return value
  
SectionEnd

Section -Debug
  
  ${if} $Debug == ${FALSE}
  
    #Check whether something went wrong
    
    ${if} $ReturnValue == "error"
  
      #Probably the file does not exist
      MessageBox MB_OK|MB_ICONSTOP "Failed to start LyX."
    
    ${elseif} $ReturnValue != 0
    
      #LyX has crashed
      MessageBox MB_YESNO|MB_ICONSTOP \
          "LyX has been closed because of an unexpected situation.$\n\
          This is most likely caused by a flaw in the software.$\n$\n\
          When you open your documents again, you will be able$\n\
          to restore an emergency save and continue working.$\n$\n\
          Would you like to view detailed information about this error?" \
          IDYES debug IDNO no_debug
  
    ${endif}
    
    no_debug:
    
      Quit
    
    debug:
    
      ShowWindow $R0 ${SW_HIDE}
    
  ${endif}
  
  ${if} $ReturnValue != 0
  
    StrCpy $ResultText "Error Information"
    StrCpy $ResultSubText "See Chapter 3 of the LyX Introduction \
        (Help > Introduction) for information about reporting this issue."
   
   ${else}
   
    StrCpy $ResultText "Debugging Completed"
    StrCpy $ResultSubText "The events you have chosen are logged below."
   
   ${endif}
  
  ${if} $Debug == ${FALSE}

    #Put the log window on the screen again
    Push "user32::SetWindowPos(i $HWNDPARENT, i 0, i 133, i 100, i 0, i 0, i ${SWP_NOSIZE})"
    CallInstDLL "$EXEDIR\System.dll" Call
    BringToFront

  ${endif}

SectionEnd


#--------------------------------
#Functions

Function InitInterface
  
  #Command line parameters
  Call GetParameters
  Pop $Parameters
  
  #Check for debug mode
  ${StrStr} $R0 $Parameters "-dbg"
  
  ${if} $R0 == ""
    StrCpy $Debug ${FALSE}
  ${else}
    StrCpy $Debug ${TRUE}
  ${endif}
  
  ${if} $Debug == ${FALSE}

    #Keep the log window outside the screen to ensure that there will be no flickering
    Push "user32::SetWindowPos(i $HWNDPARENT, i 0, i -32000, i -32000, i 0, i 0, i ${SWP_NOSIZE})"
    CallInstDLL "$EXEDIR\System.dll" Call
  
  ${endif}

FunctionEnd

Function GetLyXSetting

  #Get a LyX setting from the registry
  #First try a current user setting, then a system setting

  Exch $R0
  Push $R1

  ReadRegStr $R1 HKCU ${APP_REGKEY_SETTINGS} $R0
  
  ${if} $R1 == ""
    ReadRegStr $R1 HKLM ${APP_REGKEY_SETTINGS} $R0
  ${endif}
  
  Exch $R1
  Exch 1
  Pop $R0

FunctionEnd

Function SetEnvironmentVariable

  #Sets the value of an environment variable
  #Input on stack: name of variable, value

  Exch $R0
  Exch 1
  Exch $R1
  
  Push 'kernel32::SetEnvironmentVariable(t, t) i("$R1", "$R0")'
  CallInstDLL "$EXEDIR\System.dll" Call
  
  Pop $R1
  Pop $R0

FunctionEnd
