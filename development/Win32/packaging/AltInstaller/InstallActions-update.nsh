# this file contains the main installer section

# The '-' makes the section invisible.
Section "-Installation actions" SecInstallation

  # dummy actions to avoid NSIS warnings
  StrCpy $AspellBaseReg ""
  StrCpy $AspellMessage ""
  StrCpy $DictCode ""
  StrCpy $LangCode ""
  StrCpy $LangCodeSys ""
  StrCpy $LangName ""
  StrCpy $LangNameSys ""
  StrCpy $RunNumber ""
  
  # init, this variable is later only set to a value in function InstDirChange
  # when the $INSTDIR is changed
  StrCpy $INSTDIR_OLD ""
  
  # extract modified files
  Call UpdateModifiedFiles # macro from Updated.nsh
  
  # delete files
  Call DeleteFiles # macro from Deleted.nsh
  
  # delete old uninstaller
  Delete "${PRODUCT_UNINSTALL_EXE}"
  
  # delete old start menu folder
  ReadRegStr $0 SHCTX "${PRODUCT_UNINST_KEY_OLD}" "StartMenu"
  RMDir /r $0
  # delete desktop icon
  Delete "$DESKTOP\${PRODUCT_VERSION_OLD}.lnk"
  
  # delete old registry entries
  ${if} $CreateFileAssociations == "true"
   DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
   # remove file extension .lyx
   ReadRegStr $R0 SHCTX "Software\Classes\${PRODUCT_EXT}" ""
   ${if} $R0 == "${PRODUCT_REGNAME}"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_EXT}"
    DeleteRegKey SHCTX "Software\Classes\${PRODUCT_REGNAME}"
   ${endif}
  ${endif}
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY_OLD}"
  DeleteRegKey HKCR "Applications\lyx.bat"
  DeleteRegKey HKCR "${PRODUCT_NAME}"
  
  # determine the new name of the install location,
  # Change the old install path to the new one (currently only when the user
  # has used the default path settings of the previous LyX-version)
  Call InstDirChange
  
  # Refresh registry setings for the uninstaller
  Call RefreshRegUninst
  
  # register LyX
  ${if} $CreateFileAssociations == "true"
   WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "" "${PRODUCT_EXE}"
  ${endif}
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "RootKey" "$ProductRootKey"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "LyX ${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "${PRODUCT_UNINSTALL_EXE}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "${PRODUCT_EXE}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "${PRODUCT_INFO_URL}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_ABOUT_URL}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "Publisher" "LyX Team"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "HelpLink" "${PRODUCT_HELP_LINK}"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" 0x00000001
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" 0x00000001
  
  # create start menu entry
  SetOutPath "$INSTDIR\bin"
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "${PRODUCT_UNINSTALL_EXE}"
  
  # create desktop icon
  ${if} $CreateDesktopIcon == "true"
   SetOutPath "$INSTDIR\bin"
   CreateShortCut "$DESKTOP\LyX ${PRODUCT_VERSION}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  ${endif}
  
  # register the extension .lyx
  ${if} $CreateFileAssociations == "true"
   # write informations about file type
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}" "" "${PRODUCT_NAME} Document"
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}\DefaultIcon" "" "${PRODUCT_EXE}"
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}\Shell\open\command" "" '"${PRODUCT_BAT}" "%1"'
   # write informations about file extensions
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_EXT}" "" "${PRODUCT_REGNAME}"
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_EXT}" "Content Type" "${PRODUCT_MIME_TYPE}"  
   # refresh shell
   System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
  ${endif}
  
  # create Uninstaller
  WriteUninstaller "${PRODUCT_UNINSTALL_EXE}"
  
  # test if Python is installed
  # only use an existing python when it is version 2.5 because some Compaq and Dell PC are delivered
  # with outdated Python interpreters
  ReadRegStr $PythonPath HKLM "Software\Python\PythonCore\2.5\InstallPath" ""
  ${if} $PythonPath == ""
   StrCpy $PythonPath "$INSTDIR\bin"
  ${else}
   StrCpy $PythonPath $PythonPath -1 # remove the "\" at the end
  ${endif}
  
  # run LyX's configure script
  # create a bat-file to start configure in a console window so that the user see the progress
  # of the configuration and to have a signal when the configuration is ready to start LyX
  # this is important when LyX is installed together with MiKTeX or when LyX is installed for the first
  # time on a computer, because the installation of missing LaTeX-files required by LyX could last minutes
  # a batch file is needed because simply calling ExecWait '"$PythonPath\python.exe" "$INSTDIR\Resources\configure.py"'
  # creates the config files in $INSTDIR\bin
  StrCpy $1 $INSTDIR 2 # get drive letter
  FileOpen $R1 "$INSTDIR\Resources\configLyX.bat" w
  FileWrite $R1 '$1$\r$\n\
  		 cd $INSTDIR\Resources\$\r$\n\
  		 "$PythonPath\python.exe" configure.py'
  FileClose $R1
  MessageBox MB_OK|MB_ICONINFORMATION "$(LatexConfigInfo)"
  ExecWait '"$INSTDIR\Resources\configLyX.bat"'
  Delete "$INSTDIR\Resources\configLyX.bat"
  
  # for some unknown odd reason the folder $INSTDIR_OLD\Resources\ui
  # is not deleted in function InstDirChange, so the deletion has to be called
  # again to make it work
  ${if} $INSTDIR_OLD != ""
   RMDir /r $INSTDIR_OLD
  ${endif}

SectionEnd

# -------------------------------------------

Function InstDirChange
	
  # determine the new name of the install location,
  # Change the old install path to the new one (only when the user
  # has used the default path settings of the previous LyX-version, otherwise leave the path as is)
  StrCpy $String $INSTDIR
  StrCpy $Search "${PRODUCT_VERSION_OLD}"
  StrLen $3 $String
  Call StrPoint # search the $INSTDIR for the phrase in ${PRODUCT_VERSION_OLD} ; function from LyXUtils.nsh
  ${if} $Pointer != "-1" # if something was found
  
   IntOp $Pointer $Pointer - 1 # jump before the first "\" of "\${PRODUCT_VERSION_OLD}"
   StrCpy $String $String "$Pointer" # $String is now the part before "\${PRODUCT_VERSION_OLD}"
   # rename the installation folder by copying LyX files
   StrCpy $INSTDIR_NEW "$String\LyX ${PRODUCT_VERSION}"
   CreateDirectory "$INSTDIR_NEW"
   CopyFiles "$INSTDIR\*.*" "$INSTDIR_NEW"
   # delete the old folder
   RMDir /r $INSTDIR
   StrCpy $INSTDIR_OLD $INSTDIR
   StrCpy $INSTDIR $INSTDIR_NEW
   
   # set new path_prefix in the file lyxrc.dist
   StrCpy $OldString "${PRODUCT_VERSION_OLD}"
   StrCpy $NewString "LyX ${PRODUCT_VERSION}"
   # following macro from TextFunc.nsh # calls Function ReplaceLineContent from LyXUtils.nsh
   ${LineFind} "$INSTDIR\Resources\lyxrc.dist" "" "1:-1" "ReplaceLineContent"
   # this is only needed for this installer version 4.18 (delete it afterwards!)
    StrCpy $OldString "LyX 1.6.0"
    ${LineFind} "$INSTDIR\Resources\lyxrc.dist" "" "1:-1" "ReplaceLineContent"
   
   # set the new path to the preferences file for all users
   StrCpy $FileName "preferences"
   StrCpy $OldString "${PRODUCT_VERSION_OLD}"
   StrCpy $NewString "LyX ${PRODUCT_VERSION}"
   Call CheckAppPathPreferences # function from LyXUtils.nsh
   
   # set the new path to the session file for all users
   StrCpy $FileName "session"
   Call CheckAppPathPreferences # function from LyXUtils.nsh
   
   # set the new path to the lyx.bat file
   # following macro from TextFunc.nsh # calls Function ReplaceLineContent from LyXUtils.nsh
   ${LineFind} "$INSTDIR\bin\lyx.bat" "" "1:-1" "ReplaceLineContent" 
   
   # set new path to ImageMagick
   ReadRegStr $ImageMagickPath SHCTX "SOFTWARE\Classes\Applications" "AutoRun"
   ${if} $ImageMagickPath != ""
    ${WordReplace} $ImageMagickPath "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $ImageMagickPath # macro from WordFunc.nsh
    WriteRegStr SHCTX "SOFTWARE\Classes\Applications" "AutoRun" "$ImageMagickPath"
   ${endif}
  
  ${endif} # end ${if} $Pointer != "-1" (if the folder is renamed)
  
FunctionEnd

# -------------------------------------------

Function RefreshRegUninst

  # Refresh registry setings for the uninstaller

  # Aspell
  ReadRegStr $0 SHCTX "Software\Aspell" "OnlyWithLyX" # special entry to test if it was installed with LyX
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
  # this is only needed for this installer version 4.18 (delete it afterwards!)
   ${orif} $0 == "Yes160"
   WriteRegStr HKLM "SOFTWARE\Aspell" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
  ${endif}
  
  # Metafile2eps
  Var /GLOBAL RegLocation
  StrCpy $RegLocation "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Metafile to EPS Converter"
  ReadRegStr $0 SHCTX "$RegLocation" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
  # this is only needed for this installer version 4.18 (delete it afterwards!)
  ${orif} $0 == "Yes160"
   WriteRegStr HKLM "$RegLocation" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
   # set the new path
   ReadRegStr $0 HKLM "SOFTWARE\InkNote Selector" ""
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0 # macro from WordFunc.nsh
   WriteRegStr HKLM "SOFTWARE\InkNote Selector" "" "$0"
   ReadRegStr $0 HKLM "$RegLocation" "InstallLocation"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "$RegLocation" "InstallLocation" "$0"
   # this is only needed for this installer version 4.18 (delete it afterwards!)
    ReadRegStr $0 HKLM "SOFTWARE\InkNote Selector" ""
    ${WordReplace} $0 "LyX 1.6.0" "LyX ${PRODUCT_VERSION}" "+" $0
    WriteRegStr HKLM "SOFTWARE\InkNote Selector" "" "$0"
    ReadRegStr $0 HKLM "$RegLocation" "InstallLocation"
    ${WordReplace} $0 "LyX 1.6.0" "LyX ${PRODUCT_VERSION}" "+" $0
    WriteRegStr HKLM "$RegLocation" "InstallLocation" "$0"
  ${endif}
  
  # MiKTeX
  ReadRegStr $0 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
  # this is only needed for this installer version 4.18 (delete it afterwards!)
  ${orif} $0 == "Yes160"
   WriteRegStr HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
  ${endif}
  
  # JabRef
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
  # this is only needed for this installer version 4.18 (delete it afterwards!)
  ${orif} $0 == "Yes160"
   WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
  ${endif}
  
  # Aiksaurus currently not needed
  #ReadRegStr $0 SHCTX "Software\Aiksaurus" "OnlyWithLyX"
  #${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
  # WriteRegStr HKLM "SOFTWARE\Aiksaurus" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
  #${endif}
  
  # ImageMagick
  ReadRegStr $0 SHCTX "Software\ImageMagick" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
   WriteRegStr HKLM "SOFTWARE\ImageMagick" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
   # set the new path
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\Current" "BinPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0 # macro from WordFunc.nsh
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "BinPath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\Current" "CoderModulesPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "CoderModulesPath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\Current" "ConfigurePath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "ConfigurePath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\Current" "FilterModulesPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "FilterModulesPath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\Current" "LibPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "LibPath" "$0"
   # set the new path
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "BinPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "BinPath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "CoderModulesPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "CoderModulesPath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "ConfigurePath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "ConfigurePath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "FilterModulesPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "FilterModulesPath" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "LibPath"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion_Old}\Q:16" "LibPath" "$0"
  ${endif}
  
  # Ghostscript and GSview
  ReadRegStr $0 HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX"
  ${if} $0 == "Yes${PRODUCT_VERSION_SHORT_OLD}"
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}"
   # set the new path
   ReadRegStr $0 HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion_Old}" "GS_DLL"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion_Old}" "GS_DLL" "$0"
   ReadRegStr $0 HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion_Old}" "GS_LIB"
   ${WordReplace} $0 "${PRODUCT_VERSION_OLD}" "LyX ${PRODUCT_VERSION}" "+" $0
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion_Old}" "GS_LIB" "$0"
  ${endif}

FunctionEnd
