/*

Create uninstaller, file associations and configure LyX

*/

!define SHORTCUT '${APP_NAME} ${APP_SERIES_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"'

#--------------------------------
#Sections

Section -FileAssociations

  #Associate .lyx files with LyX for current user of all users

  #Write information about file type
  !define REG_FILETYPE 'WriteRegStr SHELL_CONTEXT "Software\Classes\${APP_REGNAME_DOC}'
  
  ${REG_FILETYPE}" "" "${APP_NAME} Document"
  ${REG_FILETYPE}\DefaultIcon" "" "$INSTDIR\bin\lyx_doc_32x32.ico"
  ${REG_FILETYPE}\Shell\open\command" "" '"$INSTDIR\${APP_RUN}" "%1"'
  
  !define REG_FILEEXT 'WriteRegStr SHELL_CONTEXT "Software\Classes\${APP_EXT}"'
  
  ${REG_FILEEXT} "" "${APP_REGNAME_DOC}"
  ${REG_FILEEXT} "Content Type" "${APP_MIME_TYPE}"  
  
  #Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'

SectionEnd

Section -InstallData

  #Registry information
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY} "" $INSTDIR
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY} "Version" "${APP_VERSION}"

  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "LaTeX Path" $PathLaTeX
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "ImageMagick Path" $PathImageMagick
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "Ghostscript Path" $PathGhostscript
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "LyX Language" $LangName

  #Start Menu shortcut
  #There is only one shortcut to the application, so it should be in the main group
  CreateShortCut "$SMPROGRAMS\${SHORTCUT}
    
  #Uninstaller information
  !define REG_UNINSTALL 'WriteRegStr SHELL_CONTEXT "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}"'
  
  ${if} $CurrentUserInstall == ${TRUE}
    ${REG_UNINSTALL} "DisplayName" "${APP_NAME} ${APP_VERSION} $(TEXT_INSTALL_CURRENTUSER)"
  ${else}
    ${REG_UNINSTALL} "DisplayName" "${APP_NAME} ${APP_VERSION}"
  ${endif}
  
  ${REG_UNINSTALL} "UninstallString" '"$INSTDIR\${SETUP_UNINSTALLER}"'
  ${REG_UNINSTALL} "DisplayVersion" "${APP_VERSION}"
  ${REG_UNINSTALL} "DisplayIcon" "$INSTDIR\bin\lyx_32x32.ico"
  ${REG_UNINSTALL} "URLUpdateInfo" "http://www.lyx.org/"
  ${REG_UNINSTALL} "URLInfoAbout" "http://www.lyx.org/about/"
  ${REG_UNINSTALL} "Publisher" "LyX Team"
  ${REG_UNINSTALL} "HelpLink" "http://www.lyx.org/internet/mailing.php"  
  
SectionEnd

Section -dvipost
  
  #Install dvipost package
  
  ${if} $PathLaTeXLocal != ""
    SetOutPath "$PathLaTeXLocal\tex\latex\dvipost"
    File "${FILES_DVIPOST_PKG}\dvipost.sty"
  ${endif}
  
  #Update file name database
  
  ${if} $PathLaTeX != ""
    nsExec::Exec '"$PathLaTeX\initexmf.exe" --update-fndb'
    Pop $R0 #Return value
  ${endif}

SectionEnd

Section -Configure

  #Windows specific configuration in lyxrc.dist
  
  Delete "$INSTDIR\Resources\lyxrc.dist"
  FileOpen $R1 "$INSTDIR\Resources\lyxrc.dist" w
  
  #Path prefix
  Call GetPathPrefix
  Pop $R0  
  FileWrite $R1 '\path_prefix "$R0"$\r$\n'
  
  #Default screen fonts
  FileWrite $R1 '\screen_font_roman "Times New Roman"$\r$\n'
  FileWrite $R1 '\screen_font_sans "Arial"$\r$\n'
  FileWrite $R1 '\screen_font_typewriter "Courier New"$\r$\n'
  FileWrite $R1 '\preview_scale_factor 1.0$\r$\n' ;Fit instant preview font size to screen fonts
  #PDF view helper
  FileWrite $R1 '\format "pdf" "pdf" "PDF (ps2pdf)" "P" "pdfview" "" "document,vector"$\r$\n'  
  FileWrite $R1 '\format "pdf2" "pdf" "PDF (pdflatex)" "F" "pdfview" "" "document,vector"$\r$\n'
  FileWrite $R1 '\format "pdf3" "pdf" "PDF (dvipdfm)" "m" "pdfview" "" "document,vector"$\r$\n'
  
  FileClose $R1
  
  #Information in the registry for the launcher
  
  #Set language
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETTINGS} "Language" $LangISOCode

SectionEnd

#--------------------------------
#Functions

Function CheckDesktopShortcut

  #Enable desktop icon creation when there is an icon already
  #Old shortcuts need to be updated
  
  ${if} ${fileexists} "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 5" "State" "1"
  ${endif}

FunctionEnd

Function CreateDesktopShortcut

  #Creating a desktop shortcut is an option on the finish page

  CreateShortCut "$DESKTOP\${SHORTCUT}

FunctionEnd
  
