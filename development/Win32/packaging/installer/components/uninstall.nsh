/*

Uninstaller

*/

;--------------------------------
;Sections

Section "un.Program Files" un.SecProgramFiles

  ;Binaries
  
  !insertmacro FileListLyXBin Delete "$INSTDIR\bin\"
  !insertmacro FileListQtBin Delete "$INSTDIR\bin\"
  !insertmacro FileListMinGWBin Delete "$INSTDIR\bin\"
  
  !ifdef BUILD_MSVC
    !insertmacro FileListDllMSVCBin Delete "$INSTDIR\bin\"
    !insertmacro FileListMSVCBin Delete "$INSTDIR\bin\"
    !insertmacro FileListMSVCManifest Delete "$INSTDIR\bin\"
  !else
    !insertmacro FileListDllMinGWBin Delete "$INSTDIR\bin\"
    !insertmacro FileListMinGWBin Delete "$INSTDIR\bin\"
  !endif
  
  !insertmacro FileListNetpbmBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDvipostBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDTLBin Delete "$INSTDIR\bin\"
  
  ;Icons
  
  !insertmacro FileListLyXIcons Delete "$INSTDIR\bin\"  
  
  ;Resources
  
  RMDir /r "$INSTDIR\Resources"
  
  ;Components of Python
  
  RMDir /r "$INSTDIR\python"
  
  ;Aiksaurus Data
  
  !insertmacro FileListAiksaurusData Delete "$INSTDIR\aiksaurus\"
  RMDir "$INSTDIR\aiksaurus"
  
  ;Shortcuts
  
  Delete "$SMPROGRAMS\${APP_NAME} ${APP_SERIES_NAME}.lnk"
  Delete "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk"

  ;Batch file and uninstaller itself
  
  Delete "$INSTDIR\${APP_RUN}"
  Delete "$INSTDIR\${SETUP_UNINSTALLER}"
  RMDir "$INSTDIR\bin"
  
  SetOutPath "$PROGRAMFILES"
  RMDir "$INSTDIR"
  
  ;Remove registry keys
  
  DeleteRegKey SHELL_CONTEXT "${APP_REGKEY_SETUP}"
  DeleteRegKey SHELL_CONTEXT "${APP_REGKEY}"
  DeleteRegKey SHELL_CONTEXT "Software\Classes\${APP_REGNAME_DOC}"
  DeleteRegKey SHELL_CONTEXT "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}"
  
  ;Remove file association
  
  ReadRegStr $R0 SHELL_CONTEXT "Software\Classes\${APP_EXT}" ""
  
  ${if} $R0 == "${APP_REGNAME_DOC}"
     DeleteRegKey SHELL_CONTEXT "Software\Classes\${APP_EXT}"
  ${endif}
  
SectionEnd

Section "un.User Preferences and Custom Files" un.SecUserFiles

  SetShellVarContext current
  RMDir /r "$APPDATA\${APP_DIR_USERDATA}"
  
SectionEnd
