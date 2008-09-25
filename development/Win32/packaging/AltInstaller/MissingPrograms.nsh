# This script contains the following functions:
#
# - MissingPrograms, (check if third-party programs are installed), uses:
#    EditorCheck
#    FileCheck
#
# - MissingProgramsPage,
#    (generate the page showing the missing programs that will be installed)
#
# - EditorCheck,
#    (test if an editor with syntax-highlighting for LaTeX-files is installed)
#
#--------------------------

Function MissingPrograms
  # check if third-party programs are installed

  # initialize variable, is later set to True when a program was not found
  ${if} $MissedProg != "True" # is already True when LaTeX is missing
   StrCpy $MissedProg "False"
  ${endif}

  # test if Ghostscript is installed
  GSloop:
  EnumRegKey $1 HKLM "Software\AFPL Ghostscript" 0
  ${if} $1 == ""
   EnumRegKey $1 HKLM "Software\GPL Ghostscript" 0
   ${if} $1 != ""
    StrCpy $2 "True"
   ${endif}
  ${endif}
  ${if} $1 != ""
   ${if} $2 == "True"
    ReadRegStr $3 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\GPL Ghostscript $1" "DisplayName"
    StrCpy $0 "Software\GPL Ghostscript\$1"
   ${else}
    ReadRegStr $3 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AFPL Ghostscript $1" "DisplayName"
    StrCpy $0 "Software\AFPL Ghostscript\$1"
   ${endif}
   ${if} $3 == "" # if nothing was found in the uninstall section
    ReadRegStr $3 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" # check if Ghostscript was installed together with LyX
   ${endif}
   ${if} $3 == "" # if nothing was found in the uninstall section
    DeleteRegKey HKLM "$0"
    goto GSloop
   ${else}
    ReadRegStr $GhostscriptPath HKLM $0 "GS_DLL"
    ${if} $GhostscriptPath != ""
     StrCpy $GhostscriptPath "$GhostscriptPath" -12 # remove ending "gsdll32.dll"
    ${else}
     StrCpy $MissedProg "True"
    ${endif}
   ${endif} # if $3
  ${else} # if $1
   StrCpy $GhostscriptPath ""
   StrCpy $MissedProg "True"
  ${endif}

  # test if Imagemagick is installed
  ReadRegStr $ImageMagickPath HKLM "Software\ImageMagick\Current" "BinPath"
  ${if} $ImageMagickPath == ""
   StrCpy $MissedProg "True"
  ${endif}

  # test if Aiksaurus is installed
  !insertmacro FileCheck $5 "meanings.dat" "${AiksaurusDir}" # macro from LyXUtils.nsh
  ${if} $5 == "True"
   StrCpy $AiksaurusPath "${AiksaurusDir}"
  ${endif}
#  ReadRegStr $AiksaurusPath HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AIK_DATA_DIR"

  # test if Aspell is installed
  StrCpy $5 ""
  ReadRegStr $5 HKCU "SOFTWARE\Aspell" "Base Path"
  ${if} $5 == ""
   ReadRegStr $5 HKLM "SOFTWARE\Aspell" "Base Path"
   StrCpy $AspellBaseReg "HKLM" # used in the aspell installation section
  ${else}
   StrCpy $AspellBaseReg "HKCU"
  ${endif}
  ${if} $5 == ""
   StrCpy $MissedProg "True"
  ${else}
   StrCpy $AspellPath "$5"
  ${endif}

  # test if Python is installed
  # only use an existing python when it is version 2.5 because many Compaq and Dell PC are delivered
  # with outdated Python interpreters
  ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.5\InstallPath" ""
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

  # test if an image editor is installed (due to LyX's bug 2654 first check for GIMP)
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

  # test if the BibTeX-editor JabRef is installed
  StrCpy $BibTeXEditorPath ""
  ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef 2.2" "UninstallString"
  ${if} $BibTeXEditorPath == ""
   ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef 2.3" "UninstallString"
  ${endif}
  ${if} $BibTeXEditorPath == ""
   ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef 2.3.1" "UninstallString"
  ${endif}
  ${if} $BibTeXEditorPath == ""
   ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef 2.4" "UninstallString"
  ${endif}
  
  # test if Inkscape is installed
  ReadRegStr $SVGPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inkscape" "InstallLocation"

FunctionEnd

# ---------------------------------------

Function MissingProgramsPage
  # generate the page showing the missing programs that will be installed

  StrCpy $0 "2" # start value for the dynamical item numbering
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "io_missing_progs.ini"
  !insertmacro MUI_HEADER_TEXT "$(MissProgHeader)" ""
  
  ${if} $MissedProg == "False"
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_missing_progs.ini" "Field 6" "Text" "$(MissProgMessage)"
  ${else}
   !insertmacro MUI_INSTALLOPTIONS_WRITE "io_missing_progs.ini" "Field 1" "Text" "$(MissProgCap)"
   ${if} ${INSTALLER_VERSION} == "Complete"
    ${if} $LatexPath == ""
     !insertmacro MUI_INSTALLOPTIONS_WRITE "io_missing_progs.ini" "Field $0" "Text" "$(MissProgLatex)"
     IntOp $0 $0 + 1
    ${endif}
   ${endif}
   ${if} $GhostscriptPath == ""
    !insertmacro MUI_INSTALLOPTIONS_WRITE "io_missing_progs.ini" "Field $0" "Text" "$(MissProgGhostscript)"
    IntOp $0 $0 + 1
   ${endif}
   ${if} $ImageMagickPath == ""
    !insertmacro MUI_INSTALLOPTIONS_WRITE "io_missing_progs.ini" "Field $0" "Text" "$(MissProgImageMagick)"
    IntOp $0 $0 + 1
   ${endif}
   ${if} $AspellPath == ""
    !insertmacro MUI_INSTALLOPTIONS_WRITE "io_missing_progs.ini" "Field $0" "Text" "$(MissProgAspell)"
   ${endif}
  ${endif}
  
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "io_missing_progs.ini"

FunctionEnd

# ---------------------------------------

Function MissingProgramsPage_LeaveFunction

 # this empty function is needed for the installer page

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

