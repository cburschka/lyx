/*

uninstall.nsh

Uninstall 

*/

Var FileAssociation

# ----------------------------------

Section "un.LyX" un.SecUnProgramFiles

  # LaTeX class files that were installed together with LyX
  # will not be uninstalled because other LyX versions will
  # need them and these few files don't harm to stay in LaTeX 
    
  # Binaries
  #!insertmacro FileListLyXBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListQtBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListDll Delete "$INSTDIR\bin\"
  #!insertmacro FileListMSVC Delete "$INSTDIR\bin\"
  #!insertmacro FileListNetpbmBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListDTLBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListDvipostBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListPDFToolsBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListPDFViewBin Delete "$INSTDIR\bin\"
  #!insertmacro FileListMetaFile2EPS Delete "$INSTDIR\bin\"
  RMDir /r "$INSTDIR\bin"

  # Resources
  RMDir /r "$INSTDIR\Resources"
  
  # Python
  RMDir /r "$INSTDIR\python"
  
  # Components of ImageMagick
  #!insertmacro FileListImageMagick Delete "$INSTDIR\imagemagick\"
  #!insertmacro FileListMSVC Delete "$INSTDIR\imagemagick\"
  RMDir /r "$INSTDIR\imagemagick"
  
  # Components of Ghostscript
  #!insertmacro FileListGhostscript Delete "$INSTDIR\ghostscript\"
  #!insertmacro FileListMSVC Delete "$INSTDIR\ghostscript\"
  RMDir /r "$INSTDIR\ghostscript"
  
  # delete start menu folder
  ReadRegStr $0 SHCTX "${APP_UNINST_KEY}" "StartMenu"
  RMDir /r "$0"
  #Delete "$SMPROGRAMS\${APP_NAME} ${APP_SERIES_NAME}.lnk"
  # delete desktop icon
  Delete "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk"
  
  # remove file extension .lyx
  ReadRegStr $0 SHCTX "${APP_DIR_REGKEY}" "OnlyWithLyX" # special entry to test if they were registered by this LyX version
  ${if} $0 == "Yes${APP_SERIES_KEY}"
   ReadRegStr $R0 SHCTX "Software\Classes\${APP_EXT}" ""
   ${if} $R0 == "${APP_REGNAME_DOC}"
    DeleteRegKey SHCTX "Software\Classes\${APP_EXT}13"
    DeleteRegKey SHCTX "Software\Classes\${APP_EXT}14"
    DeleteRegKey SHCTX "Software\Classes\${APP_EXT}15"
    DeleteRegKey SHCTX "Software\Classes\${APP_EXT}16"
   # enable this for LyX 2.1!
   # DeleteRegKey SHCTX "Software\Classes\${APP_EXT}20"
    DeleteRegKey SHCTX "Software\Classes\${APP_EXT}"
    DeleteRegKey SHCTX "Software\Classes\${APP_REGNAME_DOC}"
   ${endif}
  ${endif}
  ${if} $MultiUser.Privileges == "Admin"
   DeleteRegKey HKCR "LyX.Document"
  ${endif}

  # Uninstaller itself
  Delete "$INSTDIR\${SETUP_UNINSTALLER}"
  
  # Application folder
  SetOutPath "$TEMP"
  RMDir /r "$INSTDIR"
  
  # Registry keys
  DeleteRegKey SHCTX "${APP_REGKEY_SETUP}"
  DeleteRegKey SHCTX "${APP_REGKEY}"
  DeleteRegKey SHCTX "${APP_UNINST_KEY}"
  DeleteRegKey HKCR "Applications\lyx.exe"
  
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
  
  # clean other registry entries
  DeleteRegKey SHCTX "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\${APP_NAME}.exe"
  DeleteRegKey SHCTX "SOFTWARE\${APP_REGKEY}"
  
  # delete info that programs were installed together with LyX
  DeleteRegValue SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX"
  DeleteRegValue SHCTX "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX"  

SectionEnd

#---------------------------------
# user preferences
Section /o "un.$(UnLyXPreferencesTitle)" un.SecUnPreferences

 # remove LyX's config files
 StrCpy $AppSubfolder ${APP_DIR_USERDATA}
 Call un.DelAppPathSub # function from LyXUtils.nsh
  
SectionEnd

#---------------------------------
# MiKTeX
Section /o "un.MiKTeX" un.SecUnMiKTeX

 ${if} $LaTeXInstalled == "MiKTeX" # only uninstall MiKTeX when it was installed together with LyX 
  ReadRegStr $1 SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\MiKTeX ${MiKTeXDeliveredVersion}" "UninstallString"
  ExecWait $1 # run MiKTeX's uninstaller
 ${endif}

SectionEnd

#---------------------------------
# JabRef
Section "un.JabRef" un.SecUnJabRef

 ${if} $JabRefInstalled == "Yes" # only uninstall JabRef when it was installed together with LyX 
  ReadRegStr $1 SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "UninstallString"
  ExecWait "$1" # run JabRef's uninstaller
 ${endif}

SectionEnd

#---------------------------------
# Section descriptions
!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnMiKTeX} "$(SecUnMiKTeXDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnJabRef} "$(SecUnJabRefDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnPreferences} "$(SecUnPreferencesDescription)"
!insertmacro MUI_DESCRIPTION_TEXT ${un.SecUnProgramFiles} "$(SecUnProgramFilesDescription)"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

#Section "un.User Preferences and Custom Files" un.SecUserFiles
#
#  SetShellVarContext current
#  RMDir /r "$APPDATA\${APP_DIR_USERDATA}"
#  
#SectionEnd
