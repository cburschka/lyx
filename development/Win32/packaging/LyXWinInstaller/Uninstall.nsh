; Uninstall sections

Section "un.LyX" un.SecUnProgramFiles

  SectionIn RO

  ; delete LaTeX class files that were installed together with LyX
  FileOpen $R5 "$INSTDIR\Resources\uninstallPaths.dat" r
  FileRead $R5 $LatexPath
  FileClose $R5
  StrCpy $String $LatexPath
  StrCpy $Search "miktex\bin"
  StrLen $3 $String
  Call un.StrPoint ; search the LaTeXPath for the phrase "miktex\bin" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" ; if something was found
   IntOp $Pointer $Pointer - 1 ; jump before the first "\" of "\miktex\bin"
   StrCpy $String $String "$Pointer" ; $String is now the part before "\miktex\bin"
   Delete "$String\tex\latex\cv.cls"
   RMDir /r "$String\tex\latex\lyx"
   RMDir /r "$String\tex\latex\revtex"
   RMDir /r "$String\tex\latex\hollywood"
   RMDir /r "$String\tex\latex\broadway"
   ExecWait "$LatexPath\initexmf --update-fndb"
  ${endif}

  ; delete LyX's installation folder
  RMDir /r $INSTDIR
  ; delete start menu folder
  ReadRegStr $0 ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "StartMenu"
  RMDir /r "$0"
  ; delete desktop icon
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  ; delete registry entries
  DeleteRegKey HKCU "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCR "Applications\lyx.exe"
  DeleteRegKey HKCR "Applications\lyx.bat"
  
  ; Aiksaurus
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\Aiksaurus" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes" 
   ; unregister Aiksaurus
   ReadRegStr $1 HKLM "Software\Aiksaurus" "Data Path"
   RMDir /r "$1"
   DeleteRegKey HKLM "SOFTWARE\Aiksaurus"
  ${endif}

  ; ImageMagick
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\ImageMagick" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes" 
   ; unregister ImageMagick
   DeleteRegValue HKLM "SOFTWARE\Classes\Applications" "AutoRun"
   DeleteRegKey HKLM "SOFTWARE\ImageMagick"
  ${endif}
  
  ; Ghostscript and GSview
  StrCpy $0 ""
  StrCpy $5 ""
  ReadRegStr $0 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes"
   ; unregister Ghostscript
   DeleteRegKey HKLM "SOFTWARE\GPL Ghostscript"
   ; test if GSview is installed
   EnumRegValue $5 HKLM "Software\Ghostgum\GSview" 0
   ${if} $5 != ""
    ; unregister GSview
    MessageBox MB_ICONINFORMATION|MB_OK "$(UnGSview)"
    ReadRegStr $3 HKLM "Software\Ghostgum\GSview" "$5"
    ReadRegStr $4 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GSview $5" "UninstallString"
    ExecWait "$4"
   ${endif}
  ${endif}
  
  ; MiKTeX specific LyX setting 
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_AUTOINSTALL"
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_REPOSITORY"
  
  ; remove extension .lyx
  ${RemoveFileAssociation} "${PRODUCT_EXT}" "${PRODUCT_NAME}"
  DeleteRegKey HKCR "${PRODUCT_NAME}"
  
  ; clear info that programs were installed together with LyX
  DeleteRegValue HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  DeleteRegValue HKLM "Software\Aspell" "OnlyWithLyX"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${JabRefVersion}" "OnlyWithLyX"
  
  SetAutoClose true

SectionEnd

;---------------------------------
; user preferences
Section "un.$(UnLyXPreferencesTitle)" un.SecUnPreferences

 ; remove LyX's config files
 Call un.DelAppPathSub ; function from LyXUtils.nsh
  
SectionEnd

;---------------------------------
; Aspell
Section "un.Aspell" un.SecUnAspell

 ${if} $AspellInstallYes == "Aspell" ; only uninstall Aspell when it was installed together with LyX 
  Call un.UninstAspell ; Function from Aspell.nsh
 ${endif}

SectionEnd

;---------------------------------
; MiKTeX
Section "un.MiKTeX" un.SecUnMiKTeX

 ${if} $MiKTeXInstalled == "MiKTeX" ; only uninstall MiKTeX when it was installed together with LyX 
  ReadRegStr $1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MiKTeXDeliveredVersion}" "UninstallString"
  ExecWait "$1" ; run MiKTeX's uninstaller
 ${endif}

SectionEnd

;---------------------------------
; JabRef
Section "un.JabRef" un.SecUnJabRef

 ${if} $JabRefInstalled == "JabRef" ; only uninstall JabRef when it was installed together with LyX 
  ReadRegStr $1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${JabRefVersion}" "UninstallString"
  ExecWait "$1" ; run JabRef's uninstaller
 ${endif}

SectionEnd

;---------------------------------
; Section descriptions
!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnAspell} "$(SecUnAspellDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnMiKTeX} "$(SecUnMiKTeXDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnJabRef} "$(SecUnJabRefDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnPreferences} "$(SecUnPreferencesDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnProgramFiles} "$(SecUnProgramFilesDescription)"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

