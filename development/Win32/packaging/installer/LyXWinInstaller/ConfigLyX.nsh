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
		 start "LyX" "lyx.exe" %*$\r$\n'
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
   ${CreateApplicationAssociation} "${PRODUCT_NAME}" "${PRODUCT_NAME}" "$(FileTypeTitle)" "${PRODUCT_EXE}" "${PRODUCT_BAT}"
   ${CreateFileAssociation} "${PRODUCT_EXT}" "${PRODUCT_NAME}" "${PRODUCT_MIME_TYPE}"
  ${endif}

  ; create the LyX Application Data folder for all users
  ; this folder is automatically created by LyX when it is first started but we want to start LyX with a specific session file,
  ; so we create this folder before LyX starts and copy there the session file
  Call CreateAppPathSub ; function from LyXUtils.nsh

FunctionEnd

