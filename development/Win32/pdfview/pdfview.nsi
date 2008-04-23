/*

Windows PDF view helper
Author: Joost Verburg

This will be installed as pdfview.exe.

The application will launch the default PDF viewer to display the PDF file,
but works around the file locking problems of Adobe Reader.

Source code of pdfopen/pdfclose is available at:
http://magic.aladdin.cs.cmu.edu/2005/07/15/pdfopen-and-pdfclose/

*/

!include LogicLib.nsh
!include FileFunc.nsh

# Functions from FileFunc.nsh
!insertmacro GetParameters
!insertmacro GetFileName
!insertmacro GetParent

#--------------------------------
# Settings

Caption "PDF Viewer"
OutFile pdfview.exe
Icon "..\packaging\icons\lyx.ico"
SilentInstall silent

#--------------------------------
# Windows Vista settings

RequestExecutionLevel user

#--------------------------------
# Constants

!define FALSE 0
!define TRUE 1

!define FILE_NOTIFY_CHANGE_LAST_WRITE 0x00000010
!define WAIT_TIMEOUT 0x00000102

#--------------------------------
# Variables

Var Character
Var RunAppReturn

Var OriginalFile
Var OriginalFileName
Var OriginalDir

Var PDFFile
Var Viewer

Var ChangeNotification
Var WaitReturn
Var LockedFile

Var OriginalTimeHigh
Var OriginalTimeLow
Var CurrentTimeHigh
Var CurrentTimeLow

#--------------------------------
# Macros

!macro SystemCall STACK

  # Call a Windows API function

  Push `${STACK}`
  CallInstDLL "$EXEDIR\System.dll" Call

!macroend

!macro HideConsole COMMAND_LINE

  # Run an application and hide console output

  Push `${COMMAND_LINE}`
  CallInstDLL "$EXEDIR\Console.dll" Exec
  Pop $RunAppReturn
  
  ${If} $RunAppReturn == "error"
    MessageBox MB_OK|MB_ICONSTOP "Error opening PDF file $PDFFile."
  ${EndIf}

!macroend

#--------------------------------
# PDF viewing

Section "View PDF file"

  InitPluginsDir # Temporary directory for PDF file

  # Command line parameters
  ${GetParameters} $OriginalFile

  # Trim quotes
  StrCpy $Character $OriginalFile 1
  ${If} $Character == '"'
    StrCpy $OriginalFile $OriginalFile "" 1
  ${EndIf}
  StrCpy $Character $OriginalFile 1 -1
  ${If} $Character == '"'
    StrCpy $OriginalFile $OriginalFile -1
  ${EndIf}

  GetFullPathName $OriginalFile $OriginalFile
  ${GetFileName} $OriginalFile $OriginalFileName
  ${GetParent} $OriginalFile $OriginalDir

  SetOutPath $TEMP # The LyX tmpbuf should not be locked

  StrCpy $PDFFile $PLUGINSDIR\$OriginalFileName

  # Check whether the file will be opened with Adobe Reader or Adobe Acrobat
  !insertmacro SystemCall "shell32::FindExecutable(t '$OriginalFile', t '', t .s)"
  Call GetFileName
  Pop $Viewer

  ${If} $Viewer == ""
    MessageBox MB_OK|MB_ICONEXCLAMATION "No PDF viewer is installed. \
        Please install a PDF viewer such as Adobe Reader."
    Quit        
  ${EndIf}

  ${If} $Viewer == "AcroRd32.exe"
    ${OrIf} $Viewer == "Acrobat.exe"
    
    # Using Adobe viewer
    
    # Close existing view
    ${If} ${FileExists} $PDFFile
      !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$PDFFile"'
    ${EndIf}
    
    # Copy PDF to temporary file to allow LyX to overwrite the original
    CopyFiles /SILENT $OriginalFile $PDFFile
    
    # Open a new view
    !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --back --file "$PDFFile"'
    
    # Monitor for updates of the original file
    GetFileTime $OriginalFile $OriginalTimeHigh $OriginalTimeLow
    !insertmacro SystemCall "kernel32::FindFirstChangeNotification(t '$OriginalDir', \
      i 0, i ${FILE_NOTIFY_CHANGE_LAST_WRITE}) i.s"
    Pop $ChangeNotification
    
    ${Do}
    
      !insertmacro SystemCall "kernel32::WaitForSingleObject(i $ChangeNotification, i 10000) i.s"
      Pop $WaitReturn    
         
      # Check whether a lock is still active.
      # If not, Adode Reader is closed and we can close this application as well
      
      FileOpen $LockedFile $PDFFile a
      
      ${If} $LockedFile != ""
        # Quit this application
        FileClose $LockedFile
        Delete $PDFFile
        !insertmacro SystemCall "kernel32::FindCloseChangeNotification(i $ChangeNotification)"
        Quit
      ${EndIf}
      
      ${IfNot} $WaitReturn = ${WAIT_TIMEOUT}
        # The LyX tmpbuf has been updated
        # Check whether it's the PDF file that has been updated
          
        GetFileTime $OriginalFile $CurrentTimeHigh $CurrentTimeLow
        
        ${if} $OriginalTimeHigh != $CurrentTimeHigh
          ${orif} $OriginalTimeLow != $CurrentTimeLow
          # PDF has been modified, update view
          !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$PDFFile"'
          CopyFiles /SILENT $OriginalFile $PDFFile
          !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --back --file "$PDFFile"'
          
          # Time of new file
          StrCpy $OriginalTimeHigh $CurrentTimeHigh
          StrCpy $OriginalTimeLow  $CurrentTimeLow
        ${EndIf}
        
        #Monitor again
        !insertmacro SystemCall "kernel32::FindNextChangeNotification(i $ChangeNotification)"
        
      ${EndIf}
    
    ${Loop}
    
  ${Else}
  
    # Another PDF viewer like GSView is used
    # No need for special actions, just forward to ShellExecute
    ExecShell open $OriginalFile
    
  ${EndIf}
    
SectionEnd
