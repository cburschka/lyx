Function LatexFolder

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "io_latex.ini"
  # generate the installer page
  !insertmacro MUI_HEADER_TEXT "$(EnterLaTeXHeader1)" "$(EnterLaTeXHeader2)"
  ${if} $LatexName != ""
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 1" "Text" "$(EnterLaTeXFolder)"
  ${else}
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 1" "Text" "$(EnterLaTeXFolderNone)"
  ${endif}
  !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 2" "Text" "$(PathName)"
  ${if} $LatexPath == ""
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 3" "State" "C:\" # Does \" quote a "?
  ${else}
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 3" "State" "$LatexPath"
  ${endif}
  !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 4" "Text" "$(DontUseLaTeX)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "io_latex.ini" "Field 4" "State" "$State"
  AgainFolder:
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "io_latex.ini"
  !insertmacro MUI_INSTALLOPTIONS_READ "$State" "io_latex.ini" "Field 4" "State" # read checkbox if LaTeX should be used, "0" or not "1"
  ${if} $State == "1"
   StrCpy $LatexPath ""
   Goto ReadyFolder
  ${endif}
  !insertmacro MUI_INSTALLOPTIONS_READ "$LatexPath" "io_latex.ini" "Field 3" "State"
  # check if the latex.exe exists in the $LatexPath folder
  !insertmacro FileCheck $5 "latex.exe" "$LatexPath"
  ${if} $5 == "False"
   MessageBox MB_RETRYCANCEL "$(InvalidLaTeXFolder)" IDRETRY AgainFolder
   StrCpy $LatexPath ""
  ${endif}
  ReadyFolder:

FunctionEnd

Function LatexFolder_LeaveFunction

FunctionEnd
