# Uninstaller

# this function is called at first after starting the uninstaller
Function un.onInit

  # Check that LyX is not currently running
  FindProcDLL::FindProc "lyx.exe"
  ${if} $R0 == "1"
   MessageBox MB_OK|MB_ICONSTOP "$(UnInstallRunning)"
   Abort
  ${endif}

  # set registry root key
  StrCpy $Answer ""
  !insertmacro IsUserAdmin $Answer $UserName # macro from LyXUtils.nsh
  ${if} $Answer == "yes"
    SetShellVarContext all
  ${else}
   SetShellVarContext current
  ${endif}

  # Ascertain whether the user has sufficient privileges to uninstall.
  # abort when LyX was installed with admin permissions but the user doesn't have administrator privileges
  ReadRegStr $0 HKLM "${PRODUCT_UNINST_KEY}" "RootKey"
  ${if} $0 != ""
  ${andif} $Answer != "yes"
   MessageBox MB_OK|MB_ICONSTOP "$(UnNotAdminLabel)"
   Abort
  ${endif}
  # abort when LyX couldn't be found in the registry
  ${if} $0 == "" # check in HKCU
   ReadRegStr $0 HKCU "${PRODUCT_UNINST_KEY}" "RootKey"
   ${if} $0 == ""
     MessageBox MB_OK|MB_ICONEXCLAMATION "$(UnNotInRegistryLabel)"
   ${endif}
  ${endif}
  
  # printer settings, needed to uninstall the Metafile2eps printer
  !insertmacro PrinterInit

  # Macro to investigate name of LyX's preferences folders to be able remove them
  !insertmacro UnAppPreSuff $AppPre $AppSuff # macro from LyXUtils.nsh

  # test if Aspell was installed together with LyX
  ReadRegStr $0 SHCTX "Software\Aspell" "OnlyWithLyX" # special entry to test if it was installed with LyX
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   SectionSetText 2 "Aspell" # names the corersponding uninstaller section (has the index "2" as it is the third section in Uninstall.nsh)
   StrCpy $AspellInstallYes "Aspell"
   DeleteRegValue SHCTX "Software\Aspell" "OnlyWithLyX" # special entry to test if it was installed with LyX
  ${else}
   SectionSetText 2 "" # hides the corresponding uninstaller section
  ${endif}

  # test if MiKTeX was installed together with LyX
  ReadRegStr $0 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   SectionSetText 3 "MiKTeX" # names the corersponding uninstaller section
   StrCpy $MiKTeXInstalled "MiKTeX"
   DeleteRegValue HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${else}
   SectionSetText 3 "" # hides the corresponding uninstaller section
  ${endif}

  # test if JabRef was installed together with LyX
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   SectionSetText 4 "JabRef" # names the corersponding uninstaller section
   StrCpy $JabRefInstalled "JabRef"
   DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  ${else}
   SectionSetText 4 "" # hides the corresponding uninstaller section
  ${endif}

  # question message if the user really wants to uninstall LyX
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(UnReallyRemoveLabel)" IDYES +2 # continue if yes
  Abort

FunctionEnd

# ----------------------------------

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(UnRemoveSuccessLabel)"
  
FunctionEnd

# ----------------------------------
# Uninstall sections

Section "un.LyX" un.SecUnProgramFiles

  SectionIn RO

  # delete LaTeX class files that were installed together with LyX
  FileOpen $R5 "$INSTDIR\Resources\uninstallPaths.dat" r
  FileRead $R5 $LatexPath
  FileClose $R5
  StrCpy $String $LatexPath
  StrCpy $Search "miktex\bin"
  StrLen $3 $String
  Call un.StrPoint # search the LaTeXPath for the phrase "miktex\bin" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" # if something was found
   IntOp $Pointer $Pointer - 1 # jump before the first "\" of "\miktex\bin"
   StrCpy $String $String "$Pointer" # $String is now the part before "\miktex\bin"
   Delete "$String\tex\latex\cv.cls"
   RMDir /r "$String\tex\latex\lyx"
   RMDir /r "$String\tex\latex\revtex"
   RMDir /r "$String\tex\latex\hollywood"
   RMDir /r "$String\tex\latex\broadway"
   ExecWait "$LatexPath\initexmf --update-fndb"
  ${endif}

  # delete LyX's installation folder
  RMDir /r $INSTDIR
  # delete start menu folder
  ReadRegStr $0 SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu"
  RMDir /r "$0"
  # delete desktop icon
  Delete "$DESKTOP\LyX ${PRODUCT_VERSION}.lnk"
  
  # Aiksaurus
  !insertmacro FileCheck $5 "meanings.dat" "${AiksaurusDir}" # macro from LyXUtils.nsh
  ${if} $5 == "True"
   RMDir /r "${AiksaurusDir}"
  ${endif}
  ReadRegStr $0 SHCTX "Software\Aiksaurus" "OnlyWithLyX" # special entry to test if it was installed with LyX
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   # unregister Aiksaurus
   ReadRegStr $1 SHCTX "Software\Aiksaurus" "Data Path"
   RMDir /r "$1"
   DeleteRegKey SHCTX "SOFTWARE\Aiksaurus"
  ${endif}

  # remove file extension .lyx
  ReadRegStr $0 SHCTX "${PRODUCT_DIR_REGKEY}" "OnlyWithLyX" # special entry to test if they were registered by this LyX version
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
   ReadRegStr $R0 SHCTX "Software\Classes\${PRODUCT_EXT}" ""
   ${if} $R0 == "${PRODUCT_REGNAME}"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_EXT}13"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_EXT}14"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_EXT}15"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_EXT}"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_REGNAME}"
   ${endif}
  ${endif}
  
  # delete registry entries
  DeleteRegKey HKCU "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKCU "${PRODUCT_DIR_REGKEY_2}"
  DeleteRegKey HKCR "Applications\lyx.exe"
  DeleteRegKey HKCR "Applications\lyx.bat"

  # the following can only be done with admin permissions
  ${if} $Answer == "yes" # if admin
  
   # ImageMagick
   ReadRegStr $0 SHCTX "Software\ImageMagick" "OnlyWithLyX" # special entry to test if it was installed with LyX
   ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
    # unregister ImageMagick
    DeleteRegValue SHCTX "SOFTWARE\Classes\Applications" "AutoRun"
    DeleteRegKey SHCTX "SOFTWARE\ImageMagick"
   ${endif}
   
   # Metafile2eps
   ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Metafile to EPS Converter" "OnlyWithLyX" # special entry to test if it was installed with LyX
   ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
    # Delete printer
    ExecWait '$PrinterConf /q /dl /n "Metafile to EPS Converter"'
    # unregister Metafile2eps
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Metafile to EPS Converter"
    DeleteRegKey HKLM "Software\InkNote Selector"
   ${endif}
  
   # Ghostscript and GSview
   ReadRegStr $0 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" # special entry to test if it was installed with LyX
   ${if} $0 == "Yes${PRODUCT_VERSION_SHORT}"
    # unregister Ghostscript
    DeleteRegKey HKLM "SOFTWARE\GPL Ghostscript"
    # test if GSview is installed
    StrCpy $5 ""
    EnumRegValue $5 HKLM "Software\Ghostgum\GSview" 0
    ${if} $5 != ""
     # unregister GSview
     MessageBox MB_ICONINFORMATION|MB_OK "$(UnGSview)"
     ReadRegStr $3 HKLM "Software\Ghostgum\GSview" "$5"
     ReadRegStr $4 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GSview $5" "UninstallString"
     ExecWait "$4"
    ${endif}
   ${endif}
   
   # MiKTeX specific LyX setting !Can be deleted after LyX 1.5.0beta3!
   DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_AUTOINSTALL"
   DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_REPOSITORY"
   
  ${endif} # end if Answer (if admin)
  
  SetAutoClose true

SectionEnd

#---------------------------------
# user preferences
Section "un.$(UnLyXPreferencesTitle)" un.SecUnPreferences

 # remove LyX's config files
 StrCpy $AppSubfolder ${PRODUCT_SUBFOLDER}
 Call un.DelAppPathSub # function from LyXUtils.nsh
  
SectionEnd

#---------------------------------
# Aspell
Section "un.Aspell" un.SecUnAspell

 ${if} $AspellInstallYes == "Aspell" # only uninstall Aspell when it was installed together with LyX 
  Call un.UninstAspell # Function from Aspell.nsh
 ${endif}

SectionEnd

#---------------------------------
# MiKTeX
Section "un.MiKTeX" un.SecUnMiKTeX

 ${if} $MiKTeXInstalled == "MiKTeX" # only uninstall MiKTeX when it was installed together with LyX 
  ReadRegStr $1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MiKTeX ${MiKTeXDeliveredVersion}" "UninstallString"
  ExecWait $1 # run MiKTeX's uninstaller
 ${endif}

SectionEnd

#---------------------------------
# JabRef
Section "un.JabRef" un.SecUnJabRef

 ${if} $JabRefInstalled == "JabRef" # only uninstall JabRef when it was installed together with LyX 
  ReadRegStr $1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "UninstallString"
  ExecWait "$1" # run JabRef's uninstaller
 ${endif}

SectionEnd

#---------------------------------
# Section descriptions
!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnAspell} "$(SecUnAspellDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnMiKTeX} "$(SecUnMiKTeXDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnJabRef} "$(SecUnJabRefDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnPreferences} "$(SecUnPreferencesDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnProgramFiles} "$(SecUnProgramFilesDescription)"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

