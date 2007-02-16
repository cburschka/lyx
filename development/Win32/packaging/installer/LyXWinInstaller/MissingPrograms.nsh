Function MissingPrograms

  StrCpy $MissedProg "False"

  ; test if MiKTeX is installed
  ; read the PATH variable via the registry because NSIS' "$%Path%" variable is not updated when the PATH changes
  ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
  StrCpy $Search "miktex"
  Call LaTeXCheck ; sets the path to the latex.exe to $LatexPath ; function from LyXUtils.nsh
  ; check if MiKTeX 2.4 or 2.5 is installed
  StrCpy $String ""
  ReadRegStr $String HKLM "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Install Root"
  ${if} $String != ""
   StrCpy $MiKTeXVersion "2.4" ; needed later for the configuration of MiKTeX
   StrCpy $LaTeXName "MiKTeX 2.4"
  ${endif}
  
  ${if} $LatexPath == "" ; check if MiKTeX is installed only for the current user
   ; check for MiKTeX 2.5
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "miktex"
   Call LaTeXCheck ; function from LyXUtils.nsh
   ${if} $LatexPath != ""
    StrCpy $MiKTeXUser "HKCU" ; needed later to for a message about MiKTeX's install folder write permissions, see InstallActions-*.nsh
   ${endif}
   ; check for MiKTeX 2.4
   StrCpy $String ""
   ReadRegStr $String HKCU "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Install Root"
   ${if} $String != ""
    StrCpy $MiKTeXVersion "2.4"
    StrCpy $LaTeXName "MiKTeX 2.4"
   ${endif}
  ${endif}
  
  ${if} $LatexPath != ""
   StrCpy $MiKTeXInstalled "yes"
   ${if} $LaTeXName != "MiKTeX 2.4"
    StrCpy $LaTeXName "MiKTeX 2.5"
   ${endif} 
  ${endif}

  ; test if TeXLive is installed
  ; as described at TeXLives' homepage there should be an entry in the PATH
  ${if} $LatexPath == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
   StrCpy $Search "TeXLive"
   Call LaTeXCheck ; function from LyXUtils.nsh
  ${endif}
  ; check for the current user Path variable (the case when it is a live CD/DVD)
  ${if} $LatexPath == ""
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "texlive"
   StrCpy $2 "TeXLive"
   Call LaTeXCheck ; function from LyXUtils.nsh
  ${endif}
  ; check if the variable TLroot exists (the case when it is installed using the program "tlpmgui")
  ${if} $LatexPath == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TLroot"
   ${if} $String == ""
    ReadRegStr $String HKCU "Environment" "TLroot" ; the case when installed without admin permissions
   ${endif}
   StrCpy $LatexPath "$String\bin\win32"
   ; check if the latex.exe exists in the $LatexPath folder
   !insertmacro FileCheck $5 "latex.exe" "$LatexPath" ; macro from LyXUtils.nsh
   ${if} $5 == "False"
    StrCpy $LatexPath ""
   ${endif}
  ${endif}
  ${if} $LatexPath != ""
  ${andif} $LaTeXName != "MiKTeX 2.4"
  ${andif} $LaTeXName != "MiKTeX 2.5"
   StrCpy $LaTeXName "TeXLive"
  ${endif} 

  ; test if Ghostscript is installed
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
   ${if} $3 == "" ; if nothing was found in the uninstall section
    ReadRegStr $3 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" ; check if Ghostscript was installed together with LyX
   ${endif}
   ${if} $3 == "" ; if nothing was found in the uninstall section
    DeleteRegKey HKLM "$0"
    goto GSloop
   ${else}
    ReadRegStr $GhostscriptPath HKLM $0 "GS_DLL"
    ${if} $GhostscriptPath != ""
     StrCpy $GhostscriptPath "$GhostscriptPath" -12 ; remove ending "gsdll32.dll"
    ${else}
     StrCpy $MissedProg "True"
    ${endif}
   ${endif} ; if $3
  ${else} ; if $1
   StrCpy $GhostscriptPath ""
   StrCpy $MissedProg "True"
  ${endif}

  ; test if Imagemagick is installed
  ReadRegStr $ImageMagickPath HKLM "Software\ImageMagick\Current" "BinPath"
  ${if} $ImageMagickPath == ""
   StrCpy $MissedProg "True"
  ${endif}

  ; test if Aiksaurus is installed
  !insertmacro FileCheck $5 "meanings.dat" "${AiksaurusDir}" ; macro from LyXUtils.nsh
  ${if} $5 == "True"
   StrCpy $AiksaurusPath "${AiksaurusDir}"
  ${endif}
;  ReadRegStr $AiksaurusPath HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "AIK_DATA_DIR"

  ; test if Aspell is installed
  StrCpy $5 ""
  ReadRegStr $5 HKCU "SOFTWARE\Aspell" "Base Path"
  ${if} $5 == ""
   ReadRegStr $5 HKLM "SOFTWARE\Aspell" "Base Path"
   StrCpy $AspellBaseReg "HKLM" ; used in the aspell installation section
  ${else}
   StrCpy $AspellBaseReg "HKCU"
  ${endif}
  ${if} $5 == ""
   StrCpy $MissedProg "True"
  ${else}
   StrCpy $AspellPath "$5"
  ${endif}

  ; test if Python is installed
  ; only use an existing python when it is version 2.5 because many Compaq and Dell PC are delivered
  ; with outdated Python interpreters
  ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.5\InstallPath" ""
  ${if} $PythonPath != ""
   StrCpy $0 $PythonPath "" -1 ; remove the "\" at the end
   StrCpy $DelPythonFiles "True"
  ${endif}

  ; test if Acrobat or Adobe Reader is used as PDF-viewer
  ReadRegStr $String HKCR ".pdf" ""
  ${if} $String != "AcroExch.Document" ; this name is only used by Acrobat and Adobe Reader
   StrCpy $Acrobat "None"
  ${endif}
  ${if} $Acrobat != "None"
   ReadRegStr $String HKCR "AcroExch.Document/shell/open/command" ""
   StrCpy $Search "8" ; search for Acrobat or Adobe Reader 8 because then PDFViewWin8.exe is needed to view PDF-files
   !insertmacro StrPointer $String $Search $Pointer ; macro from LyXUtils
   ${if} $Pointer == "-1" ; if nothing was found
    StrCpy $Acrobat "7"
   ${else}
    StrCpy $Acrobat "8"
   ${endif}
  ${endif}

  ; test if a PostScript-viewer is installed, only check for GSview32
  StrCpy $PSVPath ""
  ReadRegStr $PSVPath HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\gsview32.exe" "Path"

  ; test if an editor with syntax-highlighting for LaTeX-files is installed (function in LyXUtils.nsh)
  Call EditorCheck ; function from LyXUtils.nsh

  ; test if an image editor is installed (due to LyX's bug 2654 first check for GIMP)
  StrCpy $ImageEditorPath ""
  ReadRegStr $ImageEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinGimp-2.0_is1" "DisplayIcon"
  ${if} $ImageEditorPath != ""
   StrCpy $ImageEditorPath "$ImageEditorPath" -13 ; delete "\gimp-2.x.exe"
  ${endif}
  ; check for Photoshop
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\Photoshop.exe" "Path"
  ${if} $0 != ""
   StrCpy $0 "$0" -1 ; delete the last "\"
   ${if} $ImageEditorPath != ""
    StrCpy $ImageEditorPath "$ImageEditorPath;$0"
   ${else}
    StrCpy $ImageEditorPath $0
   ${endif}
  ${endif}

  ; test if the BibTeX-editor JabRef is installed
  StrCpy $BibTeXEditorPath ""
  ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef 2.1" "UninstallString"
  ${if} $BibTeXEditorPath == ""
   ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef 2.2" "UninstallString"
  ${endif}

FunctionEnd

Function MissingProgramsPage

  ; generate the installer page - re-read empty page first
  StrCpy $0 "2"
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

Function MissingProgramsPage_LeaveFunction

FunctionEnd
