/*

uninstall.nsh

Uninstall 

*/

Var FileAssociation

Section "un.Program Files" un.SecProgramFiles

  # Binaries
  !insertmacro FileListLyXBin Delete "$INSTDIR\bin\"
  !insertmacro FileListQtBin Delete "$INSTDIR\bin\"
  !insertmacro FileListQtPlugins Delete "$INSTDIR\bin\"
  !insertmacro FileListDll Delete "$INSTDIR\bin\"
  !insertmacro FileListMSVC Delete "$INSTDIR\bin\"
  !insertmacro FileListNetpbmBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDTLBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDvipostBin Delete "$INSTDIR\bin\"
  !insertmacro FileListPDFToolsBin Delete "$INSTDIR\bin\"
  !insertmacro FileListPDFViewBin Delete "$INSTDIR\bin\"
  !insertmacro FileListMetaFile2EPS Delete "$INSTDIR\bin\"
  RMDir "$INSTDIR\bin"

  
  # Resources
  RMDir /r "$INSTDIR\Resources"
  
  # Python
  RMDir /r "$INSTDIR\python"
  
  # Components of ImageMagick
  !insertmacro FileListImageMagick Delete "$INSTDIR\imagemagick\"
  !insertmacro FileListMSVC Delete "$INSTDIR\imagemagick\"
  RMDir "$INSTDIR\imagemagick"
  
  # Components of Ghostscript
  !insertmacro FileListGhostscript Delete "$INSTDIR\ghostscript\"
  !insertmacro FileListMSVC Delete "$INSTDIR\ghostscript\"
  RMDir "$INSTDIR\ghostscript"
  
  # Shortcuts
  Delete "$SMPROGRAMS\${APP_NAME} ${APP_SERIES_NAME}.lnk"
  Delete "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk"

  # Uninstaller itself
  Delete "$INSTDIR\${SETUP_UNINSTALLER}"
  
  # Application folder
  SetOutPath "$TEMP"
  RMDir "$INSTDIR"
  
  # Registry keys
  DeleteRegKey SHELL_CONTEXT "${APP_REGKEY_SETUP}"
  DeleteRegKey SHELL_CONTEXT "${APP_REGKEY}"
  DeleteRegKey SHELL_CONTEXT "Software\Classes\${APP_REGNAME_DOC}"
  DeleteRegKey SHELL_CONTEXT "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}"
  
  # File associations
  
  ReadRegStr $FileAssociation SHELL_CONTEXT "Software\Classes\${APP_EXT}" ""
  
  ${If} $FileAssociation == "${APP_REGNAME_DOC}"
     DeleteRegKey SHELL_CONTEXT "Software\Classes\${APP_EXT}"
  ${EndIf}
  
  ${If} $MultiUser.Privileges != "Admin"
    ${OrIf} $MultiUser.Privileges != "Power"

    # Delete Postscript printer for metafile to EPS conversion
    ExecWait '$PrinterConf /q /dl /n "Metafile to EPS Converter"'

  ${EndIf}

SectionEnd

Section "un.User Preferences and Custom Files" un.SecUserFiles

  SetShellVarContext current
  RMDir /r "$APPDATA\${APP_DIR_USERDATA}"
  
SectionEnd
