/*

detection.nsh

Detection of external component locations

*/

# This script contains the following functions:
#
# - SearchExternal, calls the functions:
#    LaTeXActions
#    MissingPrograms
#    FindDictionaries
#
# - MissingPrograms, (check if third-party programs are installed), uses:
#    SEARCH_MIKTEX
#    SEARCH_TEXLIVE
#
# - FindDictionaries (finds installed spellcheck and thesaurus dictionaries)
#
# - EditorCheck,
#    (test if an editor with syntax-highlighting for LaTeX-files is installed)
#
#--------------------------

Function SearchExternal
  Call LaTeXActions # function from LaTeX.nsh
  Call MissingPrograms
  Call FindDictionaries # function from dictionaries.nsh
FunctionEnd

# ---------------------------------------

Function MissingPrograms
  # check if third-party programs are installed

  # test if Ghostscript is installed, check all cases:
  # 1. 32bit Windows
  # 2. 64bit Windows but 32bit Ghostscript
  # 3. 64bit Windows and 64bit Ghostscript
  StrCpy $3 0
  StrCpy $4 "0"
  ${if} ${RunningX64}
   SetRegView 64
  ${endif}
  # case 1. and 3.
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
  SetRegView 32
  # repeat for case 2.
  ${if} ${RunningX64}
  ${andif} $GhostscriptPath == ""
   StrCpy $3 0
   # we have to assure that we only repeat once and not forever
   ${if} $4 != "32"
    StrCpy $4 "32"
    goto GSloop
   ${endif}
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
  
  # No test necessary for Acrobat or Adobe Reader because pdfview does this job
  # each time it is called.
  
  # test if a PostScript-viewer is installed, only check for GSview
  # check all cases:
  # 1. 32bit Windows
  # 2. 64bit Windows but 32bit GSview
  # 3. 64bit Windows and 64bit GSview
  ${if} ${RunningX64}
   SetRegView 64
   StrCpy $PSVPath ""
   ReadRegStr $PSVPath HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\gsview64.exe" "Path"
   SetRegView 32
  ${endif}
  # repeat for case 1. and 2.
  ${if} $PSVPath == ""
   ReadRegStr $PSVPath HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\gsview32.exe" "Path"
  ${endif}

  # test if an editor with syntax-highlighting for LaTeX-files is installed
  Call EditorCheck

  # test if an image editor is installed
  StrCpy $ImageEditorPath ""
  # first check for Gimp which is a 64bit application on x64 Windows
  ${if} ${RunningX64}
   SetRegView 64
  ${endif}
  ReadRegStr $ImageEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" "InstallLocation"
  ${if} $ImageEditorPath != ""
   StrCpy $ImageEditorPath "$ImageEditorPathbin" # add the bin folder
   StrCpy $ImageEditor "Gimp"
  ${endif}
  ${if} ${RunningX64}
   SetRegView 32
  ${endif}
  # check for Photoshop
  ReadRegStr $0 HKLM "Software\Classes\Applications\Photoshop.exe\shell\open\command" ""
  ${if} $0 != ""
   StrCpy $0 "$0" -20 # delete '\photoshop.exe" "%1"'
   StrCpy $0 $0 "" 1 # remove the leading quote
   ${if} $ImageEditorPath != ""
    StrCpy $ImageEditorPath "$ImageEditorPath;$0"
   ${else}
    StrCpy $ImageEditorPath $0
   ${endif}
   StrCpy $ImageEditor "Photoshop"
  ${endif}
  # check for Krita
  ReadRegStr $0 HKLM "SOFTWARE\Classes\Krita.Document\shell\open\command" ""
  ${if} $0 != ""
   StrCpy $0 "$0" -16 # delete '\krita.exe" "%1"'
   StrCpy $0 $0 "" 1 # remove the leading quote
   ${if} $ImageEditorPath != ""
    StrCpy $ImageEditorPath "$ImageEditorPath;$0"
   ${else}
    StrCpy $ImageEditorPath $0
   ${endif}
   StrCpy $ImageEditor "Krita"
  ${endif}

  # test if and where the BibTeX-editor JabRef is installed
  ReadRegStr $PathBibTeXEditor HKLM "Software\JabRef" "Path"
  ${if} $PathBibTeXEditor == ""
   ReadRegStr $PathBibTeXEditor HKCU "Software\JabRef" "Path"
  ${endif}

  ${ifnot} ${FileExists} "$PathBibTeXEditor\${BIN_BIBTEXEDITOR}"
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
  ReadRegStr $SVGPath HKLM "SOFTWARE\Classes\inkscape.svg\DefaultIcon" ""
  ${if} $SVGPath != ""
   StrCpy $SVGPath $SVGPath "" 1 # remove the leading quote
   StrCpy $SVGPath $SVGPath -14 # # delete '\inkscape.exe"'
  ${endif}
  ${if} $SVGPath == ""
   # this was used before Inkscape 0.91:
   ReadRegStr $SVGPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inkscape" "InstallLocation"
  ${endif}

  # test if Gnumeric is installed
  ReadRegStr $0 HKLM "Software\Classes\Applications\gnumeric.exe\shell\Open\command" ""
  ${if} $0 != ""
   StrCpy $0 $0 -18 # remove "gnumeric.exe" "%1""
   StrCpy $0 $0 "" 1 # remove the leading quote
   StrCpy $GnumericPath $0
  ${endif}

  # test if Pandoc is installed
  # HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\John MacFarlane\Pandoc
  ${if} ${RunningX64}
   SetRegView 64 # the PATH is in the 64bit registry section
  ${endif}
  # check for the path to the pandoc.exe in Window's PATH variable
  StrCpy $5 ""
  StrCpy $Search "pandoc"
  ReadRegStr $String HKCU "Environment" "PATH"
  !insertmacro PATHCheck $5 "pandoc.exe" # macro from LyXUtils.nsh
  # if it is not in the user-specific PATH it might be in the global PATH
  ${if} $5 == "False"
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
   !insertmacro PATHCheck $5 "pandoc.exe" # macro from LyXUtils.nsh
  ${endif}
  SetRegView 32
  ${if} $5 != "False"
   StrCpy $PandocPath $5
  ${endif}

FunctionEnd

# ---------------------------------------

Function EditorCheck
  # test if an editor with syntax-highlighting for LaTeX-files is installed

  # (check for jEdit, PSPad, WinShell, ConTEXT, Crimson Editor, Vim, TeXnicCenter, LaTeXEditor, WinEdt, LEd, WinTeX)
  StrCpy $EditorPath ""
  StrCpy $0 ""
  # check for jEdit which is a 64bit application on x64 Windows
  ${if} ${RunningX64}
   SetRegView 64
  ${endif}
  ReadRegStr $EditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\jEdit_is1" "InstallLocation"
  ${if} $EditorPath != ""
   StrCpy $EditorPath $EditorPath -1 # remove "\" from the end of the string
  ${endif}
  SetRegView 32
  
  # check for PSPad
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PSPad editor_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1 # remove the "\"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  
  # check for WinShell
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinShell_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1 # remove the "\"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  
  # check for Vim which is a 64bit application on x64 Windows
  StrCpy $0 ""
  ${if} ${RunningX64}
   SetRegView 64
  ${endif}
  ReadRegStr $0 HKLM "Software\Vim\Gvim" "path"
  ${if} $0 != ""
   StrCpy $0 $0 -9 # remove "\gvim.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  SetRegView 32
  
  # check for TeXnicCenter which can be a 64bit application on x64 Windows
  StrCpy $0 ""
  ${if} ${RunningX64}
   SetRegView 64
  ${endif}
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TeXnicCenter_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1 # remove the "\"
   StrCpy $EditorPath "$EditorPath;$0"
  ${else}
   SetRegView 32
   ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TeXnicCenter_is1" "InstallLocation"
   ${if} $0 != ""
    StrCpy $0 $0 -1 # remove the "\"
    StrCpy $EditorPath "$EditorPath;$0"
   ${endif}
  ${endif}
  SetRegView 32
  
  # check for WinEdt
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinEdt 7" "InstallLocation"
  ${if} $0 != ""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}

FunctionEnd
