/*

Windows PDF view helper
Author: Joost Verburg

This will be installed as pdfview.exe.

The application will launch the default PDF viewer to display the PDF file,
but works around the file locking problems of Adobe Reader.

Source code of pdfopen/pdfclose is available at:
http://magic.aladdin.cs.cmu.edu/2005/07/15/pdfopen-and-pdfclose/

*/

!include "LogicLib.nsh"
!include "FileFunc.nsh"
!insertmacro GetParameters
!insertmacro GetFileName

;--------------------------------
;Settings

OutFile pdfview.exe
Icon "..\packaging\icons\lyx_32x32.ico"
SilentInstall silent

;--------------------------------
;Constants

!define FALSE 0
!define TRUE 1

;--------------------------------
;Variables

Var Dummy
Var FileName
Var Viewer
Var OriginalTimeHigh
Var OriginalTimeLow
Var CurrentTimeHigh
Var CurrentTimeLow

;--------------------------------
;Macros

!macro SystemCall STACK

  Push `${STACK}`
  CallInstDLL "$EXEDIR\System.dll" Call

!macroend

!macro HideConsole COMMAND_LINE

  Push `${COMMAND_LINE}`
  CallInstDLL "$EXEDIR\nsExec.dll" Exec
  Pop $Dummy
  
  ${if} $Dummy == "error"
    MessageBox MB_OK|MB_ICONSTOP "Error opening PDF file $R0."
  ${endif}

!macroend

;--------------------------------
;PDF vieweing

Section "View PDF file"

  ;Command line parameters
  Call GetParameters
  Pop $FileName

  ;Trim quotes
  StrCpy $Dummy $FileName 1
  ${if} $Dummy == '"'
    StrCpy $FileName $FileName "" 1
  ${endif}
  StrCpy $Dummy $FileName 1 -1
  ${if} $Dummy == '"'
    StrCpy $FileName $FileName -1
  ${endif}

  GetFullPathName $FileName $FileName

  ;Check whether the file will be opened with Adobe Reader or Adobe Acrobat
  Push $FileName
  !insertmacro SystemCall "shell32::FindExecutable(t s, t '', t .s)"
  Call GetFileName
  Pop $Viewer
  
  ${if} $Viewer == "AcroRd32.exe"
    ${orif} $Viewer == "Acrobat.exe"
    
    ;Using Adobe viewer
    
    ;Close existing view
    ${if} ${fileexists} $FileName~
      !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$FileName~"'
    ${endif}
    
    ;Copy PDF to temporary file to allow LyX to overwrite the original
    CopyFiles /SILENT $FileName $FileName~
    
    ;Open a new view
    !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --back --file "$FileName~"'
    
    ;Monitor for updates of the original file
    
    GetFileTime $FileName $OriginalTimeHigh $OriginalTimeLow
    
    ${do}
    
      Sleep 500
      
      FileOpen $Dummy $FileName~ a
      
      ${if} $Dummy != ""
        ;File no longer locked, reader closed
        FileClose $Dummy
        Delete $FileName~
        Quit
      ${endif}
      
      ${unless} ${fileexists} $FileName
        ;Original no longer exists
        Delete $FileName~
        Quit
      ${endif}
      
      GetFileTime $FileName $CurrentTimeHigh $CurrentTimeLow
      
      ${if} $OriginalTimeHigh != $CurrentTimeHigh
        ${orif} $OriginalTimeLow != $CurrentTimeLow
        
        ;Original has been modified, update!
        
        StrCpy $OriginalTimeHigh $CurrentTimeHigh
        StrCpy $OriginalTimeLow  $CurrentTimeLow
        !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$FileName~"'
        CopyFiles /SILENT $FileName $FileName~
        !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --back --file "$FileName~"'
        
      ${endif}
    
    ${loop}
    
  ${else}
  
    ;Another PDF viewer like GSView is used
    ;No need for special actions, just forward to ShellExecute
    ExecShell open $FileName
    
  ${endif}
    
SectionEnd
