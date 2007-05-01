Function ConfigureLyX
 ; configures LyX

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
  ${if} $PSVPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PSVPath"
  ${endif}
  ${if} $EditorPath != ""
    StrCpy $PathPrefix "$PathPrefix;$EditorPath"
  ${endif}
  ${if} $ImageEditorPath != ""
    StrCpy $PathPrefix "$PathPrefix;$ImageEditorPath"
  ${endif}
  
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
		 SET AIK_DATA_DIR=${AiksaurusDir}$\r$\n\
		 start "${PRODUCT_NAME}" "${LAUNCHER_NAME}" %*$\r$\n'
  FileClose $R1
  IfErrors 0 +2
   MessageBox MB_OK|MB_ICONEXCLAMATION "$(CreateCmdFilesFailed)"

  ; set the preferences file
  ; (having one preferences file that is modified to fit the needs is possible but not easy to maintain
  ; therefore simply delete the files that shouldn't be used)
  ; if not Acrobat or Adobe Reader is used
  ${if} $Acrobat == "None" ; clear the entries in the preferences file that define PDFViewWin7 or 8 as viewer
   Rename "$INSTDIR\Resources\preferencesGSview" "$INSTDIR\Resources\preferences"
   Delete "$INSTDIR\Resources\preferences7"
   Delete "$INSTDIR\Resources\preferences8"
  ${endif}
  ; if Acrobat or Adobe Reader is used
  ${if} $Acrobat == "7" ; used for all Acrobat (Adobe Reader) versions <= 7
   Rename "$INSTDIR\Resources\preferences7" "$INSTDIR\Resources\preferences"
   Delete "$INSTDIR\Resources\preferences8"
   Delete "$INSTDIR\Resources\preferencesGSview"
  ${endif}
  ${if} $Acrobat == "8"
   Rename "$INSTDIR\Resources\preferences8" "$INSTDIR\Resources\preferences"
   Delete "$INSTDIR\Resources\preferences7"
   Delete "$INSTDIR\Resources\preferencesGSview"
  ${endif}

  ; register LyX
  ${if} $CreateFileAssociations == "true"
   WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "${PRODUCT_EXE}"
  ${endif}
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "RootKey" "$ProductRootKey"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
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

  ; create start menu entry
  SetOutPath "$INSTDIR\bin"
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${PRODUCT_NAME}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  SetOutPath "$INSTDIR"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\Uninstall.lnk" "${PRODUCT_UNINSTALL_EXE}"

  ; create desktop icon
  ${if} $CreateDesktopIcon == "true"
   SetOutPath "$INSTDIR\bin"
   CreateShortCut "$DESKTOP\LyX ${PRODUCT_VERSION}.lnk" "${PRODUCT_BAT}" "" "${PRODUCT_EXE}"
  ${endif}

  ; register the extension .lyx
  ${if} $CreateFileAssociations == "true"
   ; write informations about file type
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}" "" "${PRODUCT_NAME} Document"
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}\DefaultIcon" "" "${PRODUCT_EXE}"
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_REGNAME}\Shell\open\command" "" '"${LAUNCHER_EXE}" "%1"'
   ; write informations about file extensions
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_EXT}" "" "${PRODUCT_REGNAME}"
   WriteRegStr SHCTX "Software\Classes\${PRODUCT_EXT}" "Content Type" "${PRODUCT_MIME_TYPE}"  
   ; refresh shell
   System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
  ${endif}

  ; create the LyX Application Data folder for all users
  ; this folder is automatically created by LyX when it is first started but we want to start LyX with a specific session file,
  ; so we create this folder before LyX starts and copy there the session file
  Call CreateAppPathSub ; function from LyXUtils.nsh

FunctionEnd

