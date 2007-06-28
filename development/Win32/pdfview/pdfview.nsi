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

#--------------------------------
#Settings

Caption "PDF Viewer"
OutFile pdfview.exe
Icon "..\packaging\icons\lyx_32x32.ico"
SilentInstall silent

#--------------------------------
#Windows Vista settings

RequestExecutionLevel user

#--------------------------------
#Constants

!define FALSE 0
!define TRUE 1

#--------------------------------
#Variables

Var Dummy
Var OriginalFile
Var OriginalFileName
Var PDFFile
Var Viewer
Var OriginalTimeHigh
Var OriginalTimeLow
Var CurrentTimeHigh
Var CurrentTimeLow

#--------------------------------
#Macros

!macro SystemCall STACK

  Push `${STACK}`
  CallInstDLL "$EXEDIR\System.dll" Call

!macroend

!macro HideConsole COMMAND_LINE

  Push `${COMMAND_LINE}`
  CallInstDLL "$EXEDIR\Console.dll" Exec
  Pop $Dummy
  
  ${if} $Dummy == "error"
    MessageBox MB_OK|MB_ICONSTOP "Error opening PDF file $R0."
  ${endif}

!macroend

#--------------------------------
#PDF vieweing

Section "View PDF file"

  InitPluginsDir #Temporary directory for PDF file

  #Command line parameters
  Call GetParameters
  Pop $OriginalFile

  #Trim quotes
  StrCpy $Dummy $OriginalFile 1
  ${if} $Dummy == '"'
    StrCpy $OriginalFile $OriginalFile "" 1
  ${endif}
  StrCpy $Dummy $OriginalFile 1 -1
  ${if} $Dummy == '"'
    StrCpy $OriginalFile $OriginalFile -1
  ${endif}

  GetFullPathName $OriginalFile $OriginalFile
  Push $OriginalFile
  Call GetFileName
  Pop $OriginalFileName

  SetOutPath $TEMP #The LyX tmpbuf should not be locked

  StrCpy $PDFFile $PLUGINSDIR\$OriginalFileName

  #Check whether the file will be opened with Adobe Reader or Adobe Acrobat
  Push $OriginalFile
  !insertmacro SystemCall "shell32::FindExecutable(t s, t '', t .s)"
  Call GetFileName
  Pop $Viewer

  ${if} $Viewer == ""
    MessageBox MB_OK|MB_ICONEXCLAMATION "No PDF viewer is installed. \
        Please install a PDF viewer such as Adobe Reader."
    Quit        
  ${endif}

  ${if} $Viewer == "AcroRd32.exe"
    ${orif} $Viewer == "Acrobat.exe"
    
    #Using Adobe viewer
    
    #Close existing view
    ${if} ${fileexists} $PDFFile
      !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$PDFFile"'
    ${endif}
    
    #Copy PDF to temporary file to allow LyX to overwrite the original
    CopyFiles /SILENT $OriginalFile $PDFFile
    
    #Open a new view
    !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --back --file "$PDFFile"'
    
    #Monitor for updates of the original file
    
    GetFileTime $OriginalFile $OriginalTimeHigh $OriginalTimeLow
    
    ${do}
    
      Sleep 500
      
      FileOpen $Dummy $PDFFile a
      
      ${if} $Dummy != ""
        #File no longer locked, reader closed
        FileClose $Dummy
        Delete $PDFFile
        Quit
      ${endif}
      
      ${if} ${fileexists} $OriginalFile
        
        GetFileTime $OriginalFile $CurrentTimeHigh $CurrentTimeLow
        
        ${if} $OriginalTimeHigh != $CurrentTimeHigh
          ${orif} $OriginalTimeLow != $CurrentTimeLow
          
          #Original has been modified, update!
          
          StrCpy $OriginalTimeHigh $CurrentTimeHigh
          StrCpy $OriginalTimeLow  $CurrentTimeLow
          !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$PDFFile"'
          CopyFiles /SILENT $OriginalFile $PDFFile
          !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --back --file "$PDFFile"'
          
        ${endif}
        
      ${endif}
    
    ${loop}
    
  ${else}
  
    #Another PDF viewer like GSView is used
    #No need for special actions, just forward to ShellExecute
    ExecShell open $OriginalFile
    
  ${endif}
    
SectionEnd
