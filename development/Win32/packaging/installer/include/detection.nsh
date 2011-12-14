/*

detection.nsh

Detection of external component locations

*/

# This script contains the following functions:
#
# - SearchExternal, calls the functions:
#    LaTeXActions
#    MissingPrograms
#
# - MissingPrograms, (check if third-party programs are installed), uses:
#    SEARCH_MIKTEX
#    SEARCH_TEXLIVE
#
# - EditorCheck,
#    (test if an editor with syntax-highlighting for LaTeX-files is installed)
#
#--------------------------

#Var ReportReturn
#Var CommandLineOutput

Function SearchExternal
  Call LaTeXActions # function from LaTeX.nsh
  Call MissingPrograms
FunctionEnd

# ---------------------------------------

Function MissingPrograms
  # check if third-party programs are installed

  # test if Ghostscript is installed
  StrCpy $3 0
  GSloop:
  EnumRegKey $1 HKLM "Software\GPL Ghostscript" $3
  ${if} $1 != ""
    ReadRegStr $2 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\GPL Ghostscript $1" "DisplayName"
    StrCpy $0 "Software\GPL Ghostscript\$1"
   ${if} $2 == "" # if nothing was found in the uninstall section
    ReadRegStr $2 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" # check if Ghostscript was installed together with LyX
   ${endif}
   ${if} $2 == "" # if nothing was found in the uninstall section
    DeleteRegKey HKLM "$0"
    goto GSloop
   ${else}
    ReadRegStr $GhostscriptPath HKLM $0 "GS_DLL"
    ${if} $GhostscriptPath != ""
     StrCpy $GhostscriptPath "$GhostscriptPath" -12 # remove ending "gsdll32.dll"
    ${endif}
    # there might be several versions installed and we want to use the newest one
    IntOp $3 $3 + 1
    goto GSloop
   ${endif} # if $2
  ${endif}

  # test if Python is installed
  # only use an existing python when it is version 2.5 or newer because some
  # older Compaq and Dell PCs were delivered with outdated Python interpreters
  # Python 3.x was reported not to work with LyX properly, see
  # http://www.lyx.org/trac/ticket/7143
  ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.5\InstallPath" ""
  ${if} $PythonPath == ""
   ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.6\InstallPath" ""
  ${endif}
  ${if} $PythonPath == ""
   ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.7\InstallPath" ""
  ${endif}
  ${if} $PythonPath != ""
   StrCpy $PythonPath $PythonPath -1 # remove the "\" at the end
   StrCpy $DelPythonFiles "True"
  ${endif}
  
  # test if Acrobat or Adobe Reader is used as PDF-viewer
  ReadRegStr $String HKCR ".pdf" ""
  ${if} $String != "AcroExch.Document" # this name is only used by Acrobat and Adobe Reader
   StrCpy $Acrobat "None"
  ${else}
   StrCpy $Acrobat "Yes"
  ${endif}

  # test if a PostScript-viewer is installed, only check for GSview32
  StrCpy $PSVPath ""
  ReadRegStr $PSVPath HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\gsview32.exe" "Path"

  # test if an editor with syntax-highlighting for LaTeX-files is installed
  Call EditorCheck

  # test if an image editor is installed
  StrCpy $ImageEditorPath ""
  ReadRegStr $ImageEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinGimp-2.0_is1" "DisplayIcon"
  ${if} $ImageEditorPath != ""
   StrCpy $ImageEditorPath "$ImageEditorPath" -13 # delete "\gimp-2.x.exe"
  ${endif}
  # check for Photoshop
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\Photoshop.exe" "Path"
  ${if} $0 != ""
   StrCpy $0 "$0" -1 # delete the last "\"
   ${if} $ImageEditorPath != ""
    StrCpy $ImageEditorPath "$ImageEditorPath;$0"
   ${else}
    StrCpy $ImageEditorPath $0
   ${endif}
  ${endif}

  # test if and where the BibTeX-editor JabRef is installed
  ReadRegStr $PathBibTeXEditor HKCU "Software\JabRef" "Path"
  ${if} $PathBibTeXEditor == ""
   ReadRegStr $PathBibTeXEditor HKLM "Software\JabRef" "Path"
  ${endif}

  ${IfNot} ${FileExists} "$PathBibTeXEditor\${BIN_BIBTEXEDITOR}"
    StrCpy $PathBibTeXEditor ""
    StrCpy $JabRefInstalled == "No"
  ${else}
   StrCpy $JabRefInstalled == "Yes"
  ${endif}
  
  # test if and where LilyPond is installed
  ReadRegStr $LilyPondPath HKLM "Software\LilyPond" "Install_Dir"
  ${if} $LilyPondPath != ""
   StrCpy $LilyPondPath "$LilyPondPath\usr\bin" # add "\usr\bin"
  ${endif}
  
  # test if Inkscape is installed
  ReadRegStr $SVGPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inkscape" "InstallLocation"
  
  # test if metafile2eps is installed
  ReadRegStr $WMFPath HKLM "Software\Microsoft\Windows NT\CurrentVersion\Print\Printers\Metafile to EPS Converter" "Name"
  
  # test if Gnumeric is installed
  ReadRegStr $0 HKLM "Software\Classes\Applications\gnumeric.exe\shell\Open\command" ""
  ${if} $0 != ""
   StrCpy $0 $0 -18 # remove "gnumeric.exe" "%1""
   StrCpy $GnumericPath $0
  ${endif}

FunctionEnd

# ---------------------------------------

Function EditorCheck
  # test if an editor with syntax-highlighting for LaTeX-files is installed

  # (check for jEdit, PSPad, WinShell, ConTEXT, Crimson Editor, Vim, TeXnicCenter, LaTeXEditor, WinEdt, LEd, WinTeX)
  StrCpy $EditorPath ""
  StrCpy $0 ""
  # check for jEdit
  ReadRegStr $EditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\jEdit_is1" "InstallLocation"
  ${if} $EditorPath != ""
   StrCpy $EditorPath $EditorPath -1 # remove "\" from the end of the string
  ${endif}
  # check for PSPad
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PSPad editor_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for WinShell
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinShell_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for ConTEXT
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ConTEXTEditor_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for Crimson Editor
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crimson Editor" "UninstallString"
  ${if} $0 != ""
   StrCpy $0 $0 -14 # remove "\uninstall.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for Vim 6.x
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Classes\Applications\gvim.exe\shell\edit\command" ""
  ${if} $0 != ""
   StrCpy $0 $0 -13 # remove "gvim.exe "%1""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for Vim 7.0
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vim 7.0" "UninstallString"
  ${if} $0 != ""
   StrCpy $0 $0 -18 # remove "\uninstall-gui.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for TeXnicCenter
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TeXnicCenter_is1" "Inno Setup: App Path"
  ${if} $0 != ""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for LaTeXEditor
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LaTeX Editor" "InstallLocation"
  ${if} $0 != ""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for WinEdt
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinEdt_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for LEd
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LEd_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  # check for WinTeX
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinTeX XP" "DisplayIcon"
  ${if} $0 != ""
   StrCpy $0 $0 -11 # remove "\wintex.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}

FunctionEnd
