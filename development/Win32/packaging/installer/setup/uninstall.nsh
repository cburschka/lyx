/*

uninstall.nsh

Uninstall 

*/

Var FileAssociation

Section "un.Program Files" un.SecProgramFiles

  # Binaries
  !insertmacro FileListLyXBin Delete "$INSTDIR\bin\"
  !insertmacro FileListLyXLauncher Delete "$INSTDIR\bin\"  
  !insertmacro FileListQtBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDll Delete "$INSTDIR\bin\"
  !insertmacro FileListMSVCBin Delete "$INSTDIR\bin\"
  !insertmacro FileListMSVCManifest Delete "$INSTDIR\bin\"
  !insertmacro FileListNetpbmBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDTLBin Delete "$INSTDIR\bin\"
  !insertmacro FileListDvipostBin Delete "$INSTDIR\bin\"
  !insertmacro FileListPDFToolsBin Delete "$INSTDIR\bin\"
  !insertmacro FileListPDFViewBin Delete "$INSTDIR\bin\"
  !insertmacro FileListNSISPluginsStandard Delete "$INSTDIR\bin\"
  !insertmacro FileListNSISPlugins Delete "$INSTDIR\bin\"
  !insertmacro FileListMetaFile2EPS Delete "$INSTDIR\bin\"
  RMDir "$INSTDIR\bin"
  
  # Resources
  RMDir /r "$INSTDIR\Resources"
  
  # Components of Python
  !insertmacro FileListPythonLibEncodings Delete "$INSTDIR\python\Lib\encodings\"
  Delete "$INSTDIR\python\Lib\encodings\*.pyc"
  RMDir "$INSTDIR\python\Lib\encodings"
  !insertmacro FileListPythonLib Delete "$INSTDIR\python\Lib\"
  Delete "$INSTDIR\python\Lib\*.pyc"  
  RMDir "$INSTDIR\python\Lib" 
  !insertmacro FileListPythonBin Delete "$INSTDIR\python\"
  !insertmacro FileListMSVCBin Delete "$INSTDIR\python\"
  !insertmacro FileListMSVCManifest Delete "$INSTDIR\python\" 
  RMDir "$INSTDIR\python"
  
  # Components of ImageMagick
  !insertmacro FileListImageMagick Delete "$INSTDIR\imagemagick\"
  !insertmacro FileListMSVCBin Delete "$INSTDIR\imagemagick\"
  !insertmacro FileListMSVCManifest Delete "$INSTDIR\imagemagick\"
  RMDir "$INSTDIR\imagemagick"
  
  # Components of Ghostscript
  !insertmacro FileListGhostscript Delete "$INSTDIR\ghostscript\"
  !insertmacro FileListMSVCBin Delete "$INSTDIR\ghostscript\"
  !insertmacro FileListMSVCManifest Delete "$INSTDIR\ghostscript\"
  RMDir "$INSTDIR\ghostscript"
  
  # Aiksaurus Data
  !insertmacro FileListAiksaurusData Delete "$INSTDIR\aiksaurus\"
  RMDir "$INSTDIR\aiksaurus"
  
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
