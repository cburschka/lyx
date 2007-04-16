; Uninstaller

; this function is called at first after starting the uninstaller
Function un.onInit

  ; Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}

  ; Ascertain whether the user has sufficient privileges to uninstall.
  SetShellVarContext current

  ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY}" "RootKey"
  ${if} $0 == ""
    ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "RootKey"
    ${if} $0 == ""
      MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)"
    ${endif}
  ${endif}

  ; If the user does *not* have administrator privileges, abort
  StrCpy $Answer ""
  !insertmacro IsUserAdmin $Answer $UserName ; macro from LyXUtils.nsh
  ${if} $Answer == "yes"
    SetShellVarContext all
  ${else}
    MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)"
    Abort
  ${endif}

  ; Macro to investigate name of LyX's preferences folders to be able remove them
  !insertmacro UnAppPreSuff $AppPre $AppSuff ; macro from LyXUtils.nsh

  ; test if Aspell was installed together with LyX
  ReadRegStr $0 HKLM "Software\Aspell" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   SectionSetText 2 "Aspell" ; names the corersponding uninstaller section (has the index "2" as it is the third section in Uninstall.nsh)
   StrCpy $AspellInstallYes "Aspell"
  ${else}
   SectionSetText 2 "" ; hides the corresponding uninstaller section
  ${endif}

  ; test if MiKTeX was installed together with LyX
  ReadRegStr $0 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   SectionSetText 3 "MiKTeX" ; names the corersponding uninstaller section
   StrCpy $MiKTeXInstalled "MiKTeX"
  ${else}
   SectionSetText 3 "" ; hides the corresponding uninstaller section
  ${endif}

  ; ignore JabRef because this could only be installed with the complete installer version
   SectionSetText 4 "" ; hides the corresponding uninstaller section
   StrCpy $JabRefInstalled ""

  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" IDYES +2
  Abort

FunctionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(UnRemoveSuccessLabel)"
  
FunctionEnd

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
  Delete "$DESKTOP\LyX ${PRODUCT_VERSION}.lnk"
  ; delete registry entries
  DeleteRegKey HKCU "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCR "Applications\lyx.exe"
  DeleteRegKey HKCR "Applications\lyx.bat"

  ; Aiksaurus
  !insertmacro FileCheck $5 "meanings.dat" "${AiksaurusDir}" ; macro from LyXUtils.nsh
  ${if} $5 == "True"
   RMDir /r "${AiksaurusDir}"
  ${endif}
;  StrCpy $0 ""
;  ReadRegStr $0 HKLM "Software\Aiksaurus" "OnlyWithLyX" ; special entry to test if it was installed with LyX
;  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}" 
;   ; unregister Aiksaurus
;   ReadRegStr $1 HKLM "Software\Aiksaurus" "Data Path"
;   RMDir /r "$1"
;   DeleteRegKey HKLM "SOFTWARE\Aiksaurus"
;  ${endif}

  ; ImageMagick
  StrCpy $0 ""
  ReadRegStr $0 HKLM "Software\ImageMagick" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}" 
   ; unregister ImageMagick
   DeleteRegValue HKLM "SOFTWARE\Classes\Applications" "AutoRun"
   DeleteRegKey HKLM "SOFTWARE\ImageMagick"
  ${endif}
  
  ; Ghostscript and GSview
  StrCpy $0 ""
  StrCpy $5 ""
  ReadRegStr $0 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" ; special entry to test if it was installed with LyX
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
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

