# LyX for Windows Launcher
# Author: Joost Verburg and Uwe Stöhr
# 
# This application will start LyX without the console.

!include "FileFunc.nsh"
!insertmacro GetParameters

Var Parameters

OutFile LyXLauncher.exe

Icon "icons\lyx_32x32.ico"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

#--------------------------------
#Version information

VIProductVersion "1.0.0.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "LyXLauncher"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Launcher for LyX - The Document Processor"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "1.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "LyX is Copyright 1995-2007 by the LyX Team"

#--------------------------------
#Main application

Section -Launch

  # hide the application window as only the window of the startet lyx.exe
  # should be visible
  HideWindow 

  # get the parameters LyX is called with: LyX-document to be opened etc.
  ${GetParameters} $Parameters # macro from FileFunc.nsh
  
  # start LyX and hide the command line window
  Push '"$EXEDIR\lyx.exe" $Parameters'
  CallInstDLL "$EXEDIR\Console.dll" ExecToLog
  
  # quit the application when LyX was closed by the user
  Quit

SectionEnd

