; The '-' makes the section invisible.
; Sections are entered in order, so the settings above are all
; available to SecInstallation
Section "-Installation actions" SecInstallation
  ; extract all files
  SetOutPath "$INSTDIR"
  File /r "${PRODUCT_SOURCEDIR}\bin"
  File /r "${PRODUCT_SOURCEDIR}\etc"
  File /r "${PRODUCT_SOURCEDIR}\Resources"

  ${if} $GhostscriptPath == ""
   ; register Ghostscript
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion}" "GS_DLL" "${GhostscriptDir}\bin\gsdll32.dll"
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion}" "GS_LIB" "${GhostscriptDir}\lib;${GhostscriptDir}\fonts;${GhostscriptDir}\Resource"
   
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" "Yes" ; special entry to tell the uninstaller that it was installed with LyX
   StrCpy $GhostscriptPath "${GhostscriptDir}\bin"
  ${else}
   ; delete unnecessary files
   RMDir /r ${GhostscriptDir}   
  ${endif}

  ${if} $ImageMagickPath == ""
   ; register ImageMagick
   WriteRegStr HKLM "SOFTWARE\Classes\Applications" "AutoRun" "${ImageMagickDir}\convert.exe $$"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "BinPath" "${ImageMagickDir}"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "CoderModulesPath" "${ImageMagickDir}\modules\coders"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "ConfigurePath" "${ImageMagickDir}\config"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "FilterModulesPath" "${ImageMagickDir}\modules\filters"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "LibPath" "${ImageMagickDir}"
   
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "BinPath" "${ImageMagickDir}"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "CoderModulesPath" "${ImageMagickDir}\modules\coders"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "ConfigurePath" "${ImageMagickDir}\config"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "FilterModulesPath" "${ImageMagickDir}\modules\filters"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "LibPath" "${ImageMagickDir}"
   WriteRegDWORD HKLM "SOFTWARE\ImageMagick\Current" "QuantumDepth" 0x00000010   
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "Version" "${ImageMagickVersion}"
   
   WriteRegStr HKLM "Software\ImageMagick" "OnlyWithLyX" "Yes" ; special entry to tell the uninstaller that it was installed with LyX
   StrCpy $ImageMagickPath ${ImageMagickDir}
  ${else}
   ; delete unnecessary files
   RMDir /r ${ImageMagickDir}
  ${endif}

  ${if} $AspellPath == ""
   ; extract Aspell's program files
   SetOutPath "$INSTDIR\external"
   File /r "${PRODUCT_SOURCEDIR}\${AspellInstall}"
   ; copy the files and register Aspell
   CopyFiles "$INSTDIR\${AspellInstall}" "$APPDATA"
   
   WriteRegStr HKLM "SOFTWARE\Aspell" "Base Path" "${AspellDir}"
   WriteRegStr HKLM "SOFTWARE\Aspell" "Dictionary Path" "${AspellDictPath}"
   WriteRegStr HKLM "SOFTWARE\Aspell" "Personal Path" "${AspellPersonalPath}"
   
   WriteRegStr HKLM "Software\Aspell" "OnlyWithLyX" "Yes" ; special entry to tell the uninstaller that it was installed with LyX
   
   WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "DisplayName" "${AspellDisplay}"
   WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "NoModify" 0x00000001
   WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "NoRepair" 0x00000001
   WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Aspell" "UninstallString" "${AspellDir}\${AspellUninstall}"
  ${endif}

  ${if} $AiksaurusPath == ""
   ; extract Aiksaurus' program files
   SetOutPath "$INSTDIR\external"
   File /r "${PRODUCT_SOURCEDIR}\${AiksaurusInstall}"
   ; copy the files and register Aiksaurus
   CopyFiles "$INSTDIR\${AiksaurusInstall}" "$APPDATA"
   WriteRegStr HKLM "Software\Aiksaurus" "OnlyWithLyX" "Yes" ; special entry to tell the uninstaller that it was installed with LyX
   WriteRegStr HKLM "Software\Aiksaurus" "Data Path" "${AiksaurusDir}"
  ${endif}

  ; create the PathPrefix
  StrCpy $PathPrefix "$INSTDIR\bin"
  ${if} $PythonPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PythonPath"
  ${endif}
  ${if} $LatexPath != ""
    StrCpy $PathPrefix "$PathPrefix;$LatexPath"
  ${endif}
  ${if} $GhostscriptPath != ""
    StrCpy $PathPrefix "$PathPrefix;$GhostscriptPath"
  ${endif}
  ${if} $ImageMagickPath != ""
    StrCpy $PathPrefix "$PathPrefix;$ImageMagickPath"
  ${endif}
  ${if} $PDFVPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PDFVPath"
  ${endif}
  ${if} $PSVPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PSVPath"
  ${endif}
  ${if} $EditorPath != ""
    StrCpy $PathPrefix "$PathPrefix;$EditorPath"
  ${endif}
  ${if} $ImageEditorPath != ""
    StrCpy $PathPrefix "$PathPrefix;$ImageEditorPath"
  ${endif}

  ; install the LaTeX class files that are delivered with LyX
  ; and enable MiKTeX's automatic package installation
  StrCpy $String $LatexPath
  StrCpy $Search "miktex\bin"
  StrLen $3 $String
  Call StrPoint ; search the LaTeXPath for the phrase "miktex\bin" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" ; if something was found
   IntOp $Pointer $Pointer - 1 ; jump before the first "\" of "\miktex\bin"
   StrCpy $String $String "$Pointer" ; $String is now the part before "\miktex\bin"
   ; install LaTeX class files
   SetOutPath "$String\tex\latex"
   File "${ClassFileDir}\cv.cls"
   CreateDirectory "$String\tex\latex\lyx"
   SetOutPath "$String\tex\latex\lyx"
   File "${ClassFileDir}\lyxchess.sty"
   File "${ClassFileDir}\lyxskak.sty"
   CreateDirectory "$String\tex\latex\revtex"
   SetOutPath "$String\tex\latex\revtex"
   File "${ClassFileDir}\revtex.cls"
   CreateDirectory "$String\tex\latex\hollywood"
   SetOutPath "$String\tex\latex\hollywood"
   File "${ClassFileDir}\hollywood.cls"
   CreateDirectory "$String\tex\latex\broadway"
   SetOutPath "$String\tex\latex\broadway"
   File "${ClassFileDir}\broadway.cls"
   ; install LaTeX-package dvipost (dvipost is not available for MiKTeX)
   SetOutPath "$String\tex\latex\"
   File /r "${DVIPostFileDir}"

   ${if} $MiKTeXVersion == "2.4"
    ; refresh MiKTeX's file name database
    ExecWait "$String\miktex\bin\initexmf --update-fndb"
    ; delete MiKTeX 2.4's dvipng executable as it is an old broken version. Then install a working one.
    Delete "$String\miktex\bin\dvipng.exe"
    ; Install a new one
    SetOutPath "$String\miktex\bin"
    File "${PRODUCT_DIR}\LyX\external\dvipng.exe"
    ; enable package installation without asking (1=Yes, 0=No, 2=Always Ask Before Installing)						    
    WriteRegStr HKCU "SOFTWARE\MiK\MiKTeX\CurrentVersion\MiKTeX" "InstallPackagesOnTheFly" "1"
    WriteRegStr HKCU "SOFTWARE\MiK\MiKTeX\CurrentVersion\MPM\Settings" "" ""
    ; Setting package repository (MiKTeX's primary package repository)
    WriteRegStr HKCU "SOFTWARE\MiK\MiKTeX\CurrentVersion\MPM" "RemotePackageRepository" "${MiKTeXRepo}"
   
   ${else} ; if MiKTeX 2.5
    ; refresh MiKTeX's file name database
    ExecWait "$LaTeXPath\initexmf --update-fndb"
    ; enable package installation without asking (t = Yes, f = No)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\2.5\MPM" "AutoInstall" "1" ; if only for curent user
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_AUTOINSTALL" "t"
    ; set package repository (MiKTeX's primary package repository)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\2.5\MPM" "RemoteRepository" "${MiKTeXRepo}" ; if only for curent user
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\2.5\MPM" "RepositoryType" "remote" ; if only for curent user
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_REPOSITORY" "${MiKTeXRepo}"
   ${endif}
   
   ; enable MiKTeX's automatic package installation
   ExecWait '$LaTeXPath\mpm.com --update-fndb'
  ${endif} ; end ${if} $Pointer

  ; install Aspell dictionaries
  ${if} $LangCode == "nb_NO"
   StrCpy $LangCode "no_NO" ; we only have a norwegian dictionary available
  ${endif}
  Call InstallAspellDictionary ; function from aspell.nsh

  ; configure LyX
  ; Set a path prefix in lyxrc.dist
  ClearErrors
  ${if} "$PathPrefix" != ""
   Delete "$INSTDIR\Resources\lyxrc.dist"
   FileOpen $R1 "$INSTDIR\Resources\lyxrc.dist" w
   FileWrite $R1 '\path_prefix "$PathPrefix"$\r$\n'
   FileClose $R1
   IfErrors 0 +2
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(ModifyingConfigureFailed)"
  ${endif}

  ; Create a batch file to start LyX with the environment variables set
  ClearErrors
  Delete "${PRODUCT_BAT}"
  FileOpen $R1 "${PRODUCT_BAT}" w
  FileWrite $R1 '@echo off$\r$\n\
    		 SET LANG=$LangCode$\r$\n\
		 start "LyX" "lyx.exe" %*$\r$\n'
  FileClose $R1
  IfErrors 0 +2
   MessageBox MB_OK|MB_ICONEXCLAMATION "$(CreateCmdFilesFailed)"

  ; set the preferences file
  ; having one preferences file that is modified to fit the needs isn't possible because the e.g. command
  ; ${LineFind} "$INSTDIR\Resources\preferences" "" "-16:-11" "DeleteLines" ; macro from TextFunc.nsh
  ; removes the file permissions for Users, so that it can later not be read (bug in ${LineFind} of TextFunc.nsh) 
  ; if not Acrobat or Adobe Reader is used
  ${if} $Acrobat == "None" ; clear the entries in the preferences file that define PDFViewWin7 or 8 as viewer
   Rename "$INSTDIR\Resources\preferencesGSview" "$INSTDIR\Resources\preferences"
   Delete "$INSTDIR\Resources\preferences7"
   Delete "$INSTDIR\Resources\preferences8"
  ${endif}
  ; if Acrobat or Adobe Reader is used
  ${if} $Acrobat == "7" ; clear the entries in the preferences file that define PDFViewWin8 as viewer
   Rename "$INSTDIR\Resources\preferences7" "$INSTDIR\Resources\preferences"
   Delete "$INSTDIR\Resources\preferences8"
   Delete "$INSTDIR\Resources\preferencesGSview"
  ${endif}
  ${if} $Acrobat == "8" ; clear the entries in the preferences filethat define PDFViewWin7 as viewer
   Rename "$INSTDIR\Resources\preferences8" "$INSTDIR\Resources\preferences"
   Delete "$INSTDIR\Resources\preferences7"
   Delete "$INSTDIR\Resources\preferencesGSview"
  ${endif}

  ; register LyX
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "${PRODUCT_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "RootKey" "$ProductRootKey"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "${PRODUCT_UNINSTALL_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "${PRODUCT_EXE}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "${PRODUCT_INFO_URL}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_ABOUT_URL}"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "LyX Team"
  WriteRegStr ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "HelpLink" "${PRODUCT_HELP_LINK}"
  WriteRegDWORD ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 0x00000001
  WriteRegDWORD ${PRODUCT_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 0x00000001

  SetOutPath "$INSTDIR\bin"
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "${PRODUCT_UNINSTALL_EXE}"

  ${if} $CreateDesktopIcon == "true"
   SetOutPath "$INSTDIR\bin"
   CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  ${endif}

  ; register the extension .lyx
  ${if} $CreateFileAssociations == "true"
   ${CreateApplicationAssociation} "${PRODUCT_NAME}" "${PRODUCT_NAME}" "$(FileTypeTitle)" "${PRODUCT_EXE}" "${PRODUCT_BAT}"
   ${CreateFileAssociation} "${PRODUCT_EXT}" "${PRODUCT_NAME}" "${PRODUCT_MIME_TYPE}"
  ${endif}

  ; create the LyX Applcation Data folder for all users
  ; this folder is automatically created by LyX when it is first started but we want to start LyX with a specific session file,
  ; so we create this folder before LyX starts and copy there the session file
  Call CreateAppPathSub ; function from LyXUtils.nsh

  ; delete unnecessary files
  ${if} $DelPythonFiles == "True"
   Delete $INSTDIR\bin\python.exe
   Delete $INSTDIR\bin\python25.dll
   Delete $INSTDIR\bin\Python-License.txt
   RMDir /r $INSTDIR\bin\Lib
  ${endif}
  RMDir /r $INSTDIR\external

  ; create Uninstaller
  WriteUninstaller "${PRODUCT_UNINSTALL_EXE}"

  ; run LyX's configure script
  ; create a bat-file to start configure in a console window so that the user see the progress
  ; of the configuration and to have a signal when the configuration is ready to start LyX
  ; this is important when LyX is installed together with MiKTeX or when LyX is installed for the first
  ; time on a computer, because the installation of missing LaTeX-files required by LyX could last minutes
  ; a batch file is needed because simply calling ExecWait '"$INSTDIR\bin\python.exe" "$INSTDIR\Resources\configure.py"'
  ; creates the config files in $INSTDIR\bin
  StrCpy $1 $INSTDIR 2 ; get drive letter
  FileOpen $R1 "$INSTDIR\Resources\configLyX.bat" w
  FileWrite $R1 'cd $INSTDIR\Resources\$\r$\n\
  		 $1$\r$\n\
		 "$INSTDIR\bin\python.exe" configure.py'
  FileClose $R1
  MessageBox MB_OK|MB_ICONINFORMATION "$(LatexConfigInfo)"
  ExecWait '"$INSTDIR\Resources\configLyX.bat"'
  Delete "$INSTDIR\Resources\configLyX.bat"

  ; ask to update MiKTeX
  ${if} $MiKTeXInstalled == "yes"
   MessageBox MB_YESNO|MB_ICONINFORMATION "$(MiKTeXInfo)" IDYES UpdateNow IDNO UpdateLater
   UpdateNow:
    StrCpy $0 $LaTeXPath -4 ; remove "\bin"
    ExecWait '"$LaTeXPath\copystart.exe" "$0\config\update.dat"' ; run MiKTeX's update wizard
   UpdateLater:
  ${endif}

  ; save MiKTeX's install path to be able to remove LyX's  LaTeX files in the uninstaller
  FileOpen $R1 "$INSTDIR\Resources\uninstallPaths.dat" w
  FileWrite $R1 '$LaTeXPath'
  FileClose $R1

SectionEnd
