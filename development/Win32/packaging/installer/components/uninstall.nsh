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
  !insertmacro FileListDllBin Delete "$INSTDIR\bin\"

  ;LaTeX Tools
  !insertmacro FileListNetpbmBin Delete "$INSTDIR\latextools\"
  !insertmacro FileListDvipostBin Delete "$INSTDIR\latextools\"
  !insertmacro FileListDTLBin Delete "$INSTDIR\latextools\"
  RMDir "$INSTDIR\latextools"
  
  ;Icons
  !insertmacro FileListLyXIcons Delete "$INSTDIR\bin\"  
  
  ;Resources
  RMDir /r "$INSTDIR\Resources"
  
  ;Components of Python
  !insertmacro FileListPythonBin Delete "$INSTDIR\python\"
  !insertmacro FileListPythonDll Delete "$INSTDIR\python\"
  !insertmacro FileListPythonDLLs Delete "$INSTDIR\python\DLLs\"
  RMDir "$INSTDIR\python\DLLs"
  !insertmacro FileListPythonLib Delete "$INSTDIR\python\Lib\"
  Delete "$INSTDIR\python\Lib\*.pyc"
  !insertmacro FileListPythonLibEncodings Delete "$INSTDIR\python\Lib\encodings\"
  Delete "$INSTDIR\python\Lib\encodings\*.pyc"
  RMDir "$INSTDIR\python\Lib\encodings"
  RMDir "$INSTDIR\python\Lib"
  !insertmacro FileListPythonLibs Delete "$INSTDIR\python\libs\"
  RMDir "$INSTDIR\python\libs"
  RMDir "$INSTDIR\python"
  
  ;Components of MSYS
  !insertmacro FileListMSYSBin Delete "$INSTDIR\shell\bin\"
  RMDir "$INSTDIR\shell\bin"
  !insertmacro FileListMSYSEtc Delete "$INSTDIR\shell\etc\"
  RMDir "$INSTDIR\shell\etc"  
  RMDir "$INSTDIR\shell"
  
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
