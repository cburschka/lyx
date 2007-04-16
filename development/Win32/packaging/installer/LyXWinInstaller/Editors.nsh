Function EditorCheck

  ; test if an editor with syntax-highlighting for LaTeX-files is installed
  ; (check for jEdit, PSPad, WinShell, ConTEXT, Crimson Editor, Vim, TeXnicCenter, LaTeXEditor, WinEdt, LEd, WinTeX)
  StrCpy $EditorPath ""
  StrCpy $0 ""
  ; check for jEdit
  ReadRegStr $EditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\jEdit_is1" "InstallLocation"
  ${if} $EditorPath != ""
   StrCpy $EditorPath $EditorPath -1 ; remove "\" from the end of the string
  ${endif}
  ; check for PSPad
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PSPad editor_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for WinShell
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinShell_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for ConTEXT
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ConTEXTEditor_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for Crimson Editor
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Crimson Editor" "UninstallString"
  ${if} $0 != ""
   StrCpy $0 $0 -14 ; remove "\uninstall.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for Vim 6.x
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Classes\Applications\gvim.exe\shell\edit\command" ""
  ${if} $0 != ""
   StrCpy $0 $0 -13 ; remove "gvim.exe "%1""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for Vim 7.0
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vim 7.0" "UninstallString"
  ${if} $0 != ""
   StrCpy $0 $0 -18 ; remove "\uninstall-gui.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for TeXnicCenter
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TeXnicCenter_is1" "Inno Setup: App Path"
  ${if} $0 != ""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for LaTeXEditor
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LaTeX Editor" "InstallLocation"
  ${if} $0 != ""
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for WinEdt
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinEdt_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for LEd
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LEd_is1" "InstallLocation"
  ${if} $0 != ""
   StrCpy $0 $0 -1
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}
  ; check for WinTeX
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinTeX XP" "DisplayIcon"
  ${if} $0 != ""
   StrCpy $0 $0 -11 ; remove "\wintex.exe"
   StrCpy $EditorPath "$EditorPath;$0"
  ${endif}

FunctionEnd
