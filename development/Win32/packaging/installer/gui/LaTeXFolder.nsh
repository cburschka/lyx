Function LatexFolder

  !insertmacro INSTALLOPTIONS_EXTRACT_AS "gui\io_latex.ini" "io_latex.ini"
  # generate the installer page
  !insertmacro MUI_HEADER_TEXT "$(EnterLaTeXHeader1)" "$(EnterLaTeXHeader2)"
  ${if} $LatexName != ""
   !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 1" "Text" "$(EnterLaTeXFolder)"
  ${else}
   !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 1" "Text" "$(EnterLaTeXFolderNone)"
  ${endif}
  !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 2" "Text" "$(PathName)"
  ${if} $PathLaTeX == ""
   !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 3" "State" "C:\"
  ${else}
   !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 3" "State" "$PathLaTeX"
  ${endif}
  !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 4" "Text" "$(DontUseLaTeX)"
  !insertmacro INSTALLOPTIONS_WRITE "io_latex.ini" "Field 4" "State" "$State"
  AgainFolder:
  !insertmacro INSTALLOPTIONS_DISPLAY "io_latex.ini"
  !insertmacro INSTALLOPTIONS_READ "$State" "io_latex.ini" "Field 4" "State" # read checkbox if LaTeX should be used, "0" or not "1"
  ${if} $State == "1"
   StrCpy $PathLaTeX ""
   Goto ReadyFolder
  ${endif}
  !insertmacro INSTALLOPTIONS_READ "$PathLaTeX" "io_latex.ini" "Field 3" "State"
  # check if the latex.exe exists in the $PathLaTeX folder
  !insertmacro FileCheck $5 "latex.exe" "$PathLaTeX"
  ${if} $5 == "False"
   MessageBox MB_RETRYCANCEL "$(InvalidLaTeXFolder)" IDRETRY AgainFolder
   StrCpy $PathLaTeX ""
  ${endif}
  ReadyFolder:

FunctionEnd

Function LatexFolder_LeaveFunction

FunctionEnd
