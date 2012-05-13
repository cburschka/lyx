/*

detection.nsh

Detection of external component locations

*/

Function SearchExternal
  Call SearchLaTeX
  Call SearchBibTeXEditor
  Call SearchGnumeric
FunctionEnd

#--------------------------------
# MiKTeX

Var ReportReturn
Var CommandLineOutput

!macro SEARCH_MIKTEX25

  # Search location of MiKTeX installation using initexmf
  # Works for version 2.5 and later
  
  nsExec::ExecToStack "initexmf.exe --report"
  Pop $ReportReturn
  Pop $CommandLineOutput

  ClearErrors
  ${WordFind2X} $CommandLineOutput "BinDir: " "$\r" "E+1" $PathLaTeX
  ${If} ${Errors}
    ClearErrors
    ${WordFind2X} $CommandLineOutput "CommonInstall: " "$\r" "E+1" $PathLaTeX
    ${If} ${Errors}
       StrCpy $PathLaTeX ""
    ${Else}
       StrCpy $PathLaTeX "$PathLaTeX\miktex\bin"
    ${EndIf}
  ${EndIf}

  ClearErrors
  ${WordFind2X} $CommandLineOutput "BinDir: " "$\r" "E+1" $PathLaTeX
  ${If} ${Errors}
    StrCpy $PathLaTeX ""
  ${EndIf}

  ${If} $PathLatex == ""
    ClearErrors
    ${WordFind2X} $CommandLineOutput "CommonInstall: " "$\r" "E+1" $PathLaTeX
    ${If} ${Errors}
      StrCpy $PathLaTeX ""
    ${Else}
       StrCpy $PathLaTeX "$PathLaTeX\miktex\bin"
    ${EndIf}
  ${EndIf}

  ${If} $PathLatex == ""
    ClearErrors
    ${WordFind2X} $CommandLineOutput "UserInstall: " "$\r" "E+1" $PathLaTeX
    ${If} ${Errors}
      StrCpy $PathLaTeX ""
    ${Else}
       StrCpy $PathLaTeX "$PathLaTeX\miktex\bin"
    ${EndIf}
  ${EndIf}

  # Local root

  ClearErrors
  ${WordFind2X} $CommandLineOutput "CommonData: " "$\r" "E+1" $PathLaTeXLocal
  ${If} ${Errors}
    StrCpy $PathLaTeXLocal ""
  ${EndIf}

  ${If} $PathLatex == ""
    ClearErrors
    ${WordFind2X} $CommandLineOutput "UserData: " "$\r" "E+1" $PathLaTeXLocal
    ${If} ${Errors}
      StrCpy $PathLaTeXLocal ""
    ${EndIf}
  ${EndIf}

!macroend

Function SearchLaTeX

  # Search where MikTeX is installed
  
  !insertmacro SEARCH_MIKTEX25

  ${IfNot} ${FileExists} "$PathLaTeX\${BIN_LATEX}"
    StrCpy $PathLateX ""
  ${EndIf}

FunctionEnd

#--------------------------------
# JabRef

Function SearchBibTeXEditor

  # Search where JabRef is installed
  ReadRegStr $PathBibTeXEditor HKCU "Software\JabRef" "Path"

  ${IfNot} ${FileExists} "$PathBibTeXEditor\${BIN_BIBTEXEDITOR}"
    ReadRegStr $PathBibTeXEditor HKLM "Software\JabRef" "Path"
  ${EndIf}

  ${IfNot} ${FileExists} "$PathBibTeXEditor\${BIN_BIBTEXEDITOR}"
    StrCpy $PathBibTeXEditor ""  
  ${EndIf}

FunctionEnd


#--------------------------------
# Gnumeric

Function SearchGnumeric

  ReadRegStr $PathGnumeric HKCU "Software\GNOME\Gnumeric" "Path"

  ${IfNot} ${FileExists} "$PathGnumeric\gnumeric.exe"
    ReadRegStr $PathGnumeric HKLM "Software\GNOME\Gnumeric" "Path"
  ${EndIf}

  ${IfNot} ${FileExists} "$PathGnumeric\gnumeric.exe"
    ReadRegStr $0 HKLM "Software\Classes\Applications\gnumeric.exe\shell\Open\command" ""
    ${If} $0 != ""
      StrCpy $0 $0 -18 # remove \"gnumeric.exe" "%1"\"
      StrCpy $PathGnumeric $0
    ${EndIf}
  ${EndIf}

FunctionEnd

