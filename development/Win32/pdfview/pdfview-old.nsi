/*

Windows PDF view helper
Author: Joost Verburg and Uwe Stöhr

This will be installed as pdfview.exe.

The application will launch the default PDF viewer to display the PDF file,
but works around the file locking problems of Adobe Reader and Acrobat.

The files pdfopen/pdfclose are part of this archive:
http://www.tex.ac.uk/tex-archive/systems/win32/w32tex/pdftex-w32.tar.xz

*/

!include LogicLib.nsh
!include FileFunc.nsh

#--------------------------------
# Settings

Caption "PDF Viewer"
OutFile pdfview-old.exe
Icon "..\packaging\icons\lyx.ico"
SilentInstall silent

#--------------------------------
# Windows Vista (and later) settings

RequestExecutionLevel user

#--------------------------------
# Constants

!define FALSE 0
!define TRUE 1

# see http://msdn.microsoft.com/en-us/library/windows/desktop/aa364417%28v=vs.85%29.aspx
!define FILE_NOTIFY_CHANGE_LAST_WRITE 0x00000010
# see http://msdn.microsoft.com/en-us/library/windows/desktop/ms687032%28v=vs.85%29.aspx
!define WAIT_TIMEOUT 0x00000102

#--------------------------------
# Variables

Var Character
Var RunAppReturn

Var OriginalFile
Var OriginalFileName
Var OriginalDir

Var PDFFile
Var ViewerFileName
Var Viewer

Var ChangeNotification
Var WaitReturn
Var LockedFile

Var TimeDiff

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

# all following macros and functions are from
# http://nsis.sourceforge.net/FileTimeDiff
!define GetFileTimeS "!insertmacro _GetFileTimeS"

!macro _GetFileTimeS _Time_ _File_

   Push "${_File_}"
   Call GetFileTimeS
   Pop ${_Time_}
   
!macroend

Function GetFileTimeS
	
   Exch $0  # File / hi
   Push $1  # lo
 
   ClearErrors
   GetFileTime "$0" $0 $1
   IfErrors err
   System::Call '*(i r1, i r0) i .r0'
   System::Call '*$0(l .r0)'
   System::Int64Op $0 / 10000000  # Conversion From '100 ns' to '1 sec' unit
   Goto end
 
   err:
      Push ""
      SetErrors
      Goto +3
   end:
   System::Free $0
   Exch 2
   Pop $0
   Pop $1
   
FunctionEnd

!define FileTimeDiff "!insertmacro _FileTimeDiff"

!macro _FileTimeDiff _RetVal_ _FileA_ _FileB_

   Push "${_FileB_}"
   Push "${_FileA_}"
   Call FileTimeDiff
   Pop ${_RetVal_}
   
!macroend

Function FileTimeDiff 
   Exch $0  # FileA
   Exch 
   Exch $1  # FileB
 
   ${GetFileTimeS} $0 "$0"
   IfErrors err
   ${GetFileTimeS} $1 "$1"
   IfErrors err
   System::Int64Op $0 - $1
   Goto end
 
   err:
      Push ""
      SetErrors
   end:
   Exch 2
   Pop $0
   Pop $1
   
FunctionEnd

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
  ${GetParent} $OriginalFile $OriginalDir # tmpbuf
  ${GetParent} $OriginalDir $OriginalDir # tmpdir

  SetOutPath $TEMP # The LyX tmpbuf should not be locked

  StrCpy $PDFFile $PLUGINSDIR\$OriginalFileName

  # Check whether the file will be opened with Adobe Reader or Adobe Acrobat
  !insertmacro SystemCall "shell32::FindExecutable(t '$OriginalFile', t '', t .s)"
  Pop $ViewerFileName
  ${GetFileName} $ViewerFileName $Viewer

  ${If} $Viewer == ""
    MessageBox MB_OK|MB_ICONEXCLAMATION "No PDF viewer is installed. \
        Please install a PDF viewer such as Adobe Reader."
    Quit        
  ${EndIf}

  ${If} $Viewer == "AcroRd32.exe"
  ${OrIf} $Viewer == "Acrobat.exe"
  
    # get the version of Acrobat - currenly not necessary
    #GetDLLVersion $ViewerFileName $R0 $R1
    #IntOp $R2 $R0 >> 16
    #IntOp $R2 $R2 & 0x0000FFFF ; $R2 now contains major version
    #IntOp $R3 $R0 & 0x0000FFFF ; $R3 now contains minor version
    #IntOp $R4 $R1 >> 16
    #IntOp $R4 $R4 & 0x0000FFFF ; $R4 now contains release
    #IntOp $R5 $R1 & 0x0000FFFF ; $R5 now contains build
    #StrCpy $ViewerVersion "$R2"
    
    # Close existing view
    ${If} ${FileExists} $PDFFile
      !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$PDFFile"'
    ${EndIf}
    
    # Copy PDF to temporary file to allow LyX to overwrite the original
    CopyFiles /SILENT $OriginalFile $PDFFile
    
    # Open a new view
    !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --file "$PDFFile"'
        
    # check if a file in LyX's temp folder has been changed
    !insertmacro SystemCall "kernel32::FindFirstChangeNotification(t '$OriginalDir', \
      i 1, i ${FILE_NOTIFY_CHANGE_LAST_WRITE}) i.s"
    Pop $ChangeNotification
    
    ${Do}
    
      # wait until the folder is not changed anymore, if so a "0" is returned
      # otherwise a "258" (0x00000102) is returned
      !insertmacro SystemCall "kernel32::WaitForSingleObject(i $ChangeNotification, i 10000) i.s"
      Pop $WaitReturn
      
      # Check whether the lock of the PDF file is still active (if not, Adobe Reader is closed)
      FileOpen $LockedFile $PDFFile a
      ${If} $LockedFile != ""
        # Quit this application
        FileClose $LockedFile
        Delete $PDFFile
        !insertmacro SystemCall "kernel32::FindCloseChangeNotification(i $ChangeNotification)"
        Quit
      ${EndIf}
      
      # if the folder is (for now) not changed anymore
      ${IfNot} $WaitReturn = ${WAIT_TIMEOUT}
      
        # check if the PDF-file in our temp directory is older than the one
        # in LyX's temp folder because then it has been changed by LaTeX
        ${FileTimeDiff} $TimeDiff "$PDFFile" "$OriginalFile"
        
        # if the file is older than 1 second
        ${If} $TimeDiff < -1
          # close the PDF
          !insertmacro HideConsole '"$EXEDIR\pdfclose.exe" --file "$PDFFile"'
          
          # The problem is now that LaTeX might need several runs and therefore the PDF can
          # also be rewritten consecutively several times.
          # If we would directly open the file we will get in troubles as the PDF can be
          # unreadable. We also don't know the time of a LaTeX run.
          # (As example take UserGuide.lyx, view it, then remove a letter in a section heading
          # and finally update the view.)
          # We therefore loop until the PDF is no longer changed and wait some time in each loop.
          ${Do}
           CopyFiles /SILENT $OriginalFile $PDFFile
           # wait 1.666 seconds (is empirically enough time that the PDF can be changed)
           Sleep 1666
           ${FileTimeDiff} $TimeDiff "$PDFFile" "$OriginalFile"
          ${LoopUntil} $TimeDiff = 0
          
          # open the new file
          !insertmacro HideConsole '"$EXEDIR\pdfopen.exe" --file "$PDFFile"'
        ${EndIf}
        
        #Monitor again
        !insertmacro SystemCall "kernel32::FindNextChangeNotification(i $ChangeNotification)"
        
      ${EndIf} # end ifnot
    
    ${Loop}
    
  ${Else}
  
    # Another PDF viewer like GSView is used
    # No need for special actions, just forward to ShellExecute
    ExecShell open $OriginalFile
    
  ${EndIf}
    
SectionEnd
