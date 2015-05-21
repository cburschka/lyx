/*

Windows PDF view helper
Author: Uwe Stöhr and Joost Verburg

This will be installed as pdfview.exe.

The application will launch the default PDF viewer to display the PDF file,
but works around the file locking problems of Adobe Reader and Acrobat.

*/

!include LogicLib.nsh
!include FileFunc.nsh

#--------------------------------
# Settings

Caption "PDF Viewer"
OutFile pdfview.exe
Icon "..\packaging\icons\lyx.ico"
SilentInstall silent

#--------------------------------
# Windows Vista (and later) settings

RequestExecutionLevel user

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
Var ViewerHandle
Var ViewerVersion

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
  
  ${if} $Viewer == "AcroRd32.exe"
  ${orif} $Viewer == "AcroRd64.exe"
  ${orif} $Viewer == "Acrobat.exe"
    
    # get the version of Acrobat - currenly not necessary
    GetDLLVersion $ViewerFileName $R0 $R1
    IntOp $R2 $R0 >> 16
    IntOp $R2 $R2 & 0x0000FFFF ; $R2 now contains major version
    #IntOp $R3 $R0 & 0x0000FFFF ; $R3 now contains minor version
    #IntOp $R4 $R1 >> 16
    #IntOp $R4 $R4 & 0x0000FFFF ; $R4 now contains release
    #IntOp $R5 $R1 & 0x0000FFFF ; $R5 now contains build
    StrCpy $ViewerVersion $R2
    
    # check if there is a windows open containing the PDF
    ${if} $Viewer == "AcroRd32.exe"
    ${orif} $Viewer == "AcroRd64.exe"
     ${if} $ViewerVersion > 14
      FindWindow $ViewerHandle "" "$OriginalFileName - Adobe Acrobat Reader DC"
     ${else}
      FindWindow $ViewerHandle "" "$OriginalFileName - Adobe Reader"
     ${endif}
    ${elseif} $Viewer == "Acrobat.exe"
     FindWindow $ViewerHandle "" "$OriginalFileName - Adobe Acrobat"
     ${if} $ViewerHandle == "0"
      FindWindow $ViewerHandle "" "$OriginalFileName - Adobe Acrobat Pro"
     ${endif}
    ${endif}
    ${if} $ViewerHandle <> "0" # close the window
      SendMessage $ViewerHandle 16 0 0
    ${endif}
    
    # Copy PDF to temporary file to allow LyX to overwrite the original
    CopyFiles /SILENT $OriginalFile $PDFFile
    
    # Open a new view
    !insertmacro HideConsole '"$ViewerFileName" "$PDFFile"'
    
  ${Else}
  
    # Another PDF viewer like GSView is used
    # No need for special actions, just forward to ShellExecute
    ExecShell open $OriginalFile
    
  ${EndIf}
    
SectionEnd
