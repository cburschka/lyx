/*

configure.nsh

Write registry information and configure LyX

*/

Var DistFile
Var PathPrefix

!define SHORTCUT '${APP_NAME} ${APP_SERIES_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"'

#--------------------------------
# File associations

Section -FileAssociations

  # Associate .lyx files with LyX for current user of all users

  # Write information about file type
  !define REG_FILETYPE 'WriteRegStr SHELL_CONTEXT "Software\Classes\${APP_REGNAME_DOC}'
  
  ${REG_FILETYPE}" "" "${APP_NAME} Document"
  ${REG_FILETYPE}\DefaultIcon" "" "$INSTDIR\bin\lyx.exe,0"
  ${REG_FILETYPE}\Shell\open\command" "" '"$INSTDIR\${APP_RUN}" "%1"'
  
  !define REG_FILEEXT 'WriteRegStr SHELL_CONTEXT "Software\Classes\${APP_EXT}"'
  
  ${REG_FILEEXT} "" "${APP_REGNAME_DOC}"
  ${REG_FILEEXT} "Content Type" "${APP_MIME_TYPE}"  
  
  # Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'

SectionEnd

#--------------------------------
# Registry information

Section -InstallData

  # Registry information
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY} "" $INSTDIR
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY} "Version" "${APP_VERSION}"

  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "LaTeX Path" $PathLaTeX
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "ImageMagick Path" $PathImageMagick
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "Ghostscript Path" $PathGhostscript
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "LyX Language" $LangName

  # Start Menu shortcut
  # There is only one shortcut to the application, so it should be in the main group
  CreateShortCut "$SMPROGRAMS\${SHORTCUT}
    
  # Uninstaller information
  !define REG_UNINSTALL 'WriteRegStr SHELL_CONTEXT "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SETUP_UNINSTALLER_KEY}"'
  
  ${If} $MultiUser.InstallMode == "CurrentUser"
    ${REG_UNINSTALL} "DisplayName" "${APP_NAME} ${APP_VERSION} $(TEXT_INSTALL_CURRENTUSER)"
  ${Else}
    ${REG_UNINSTALL} "DisplayName" "${APP_NAME} ${APP_VERSION}"
  ${EndIf}
  
  ${REG_UNINSTALL} "UninstallString" '"$INSTDIR\${SETUP_UNINSTALLER}"'
  ${REG_UNINSTALL} "DisplayVersion" "${APP_VERSION}"
  ${REG_UNINSTALL} "DisplayIcon" "$INSTDIR\bin\lyx.exe,0"
  ${REG_UNINSTALL} "URLUpdateInfo" "http://www.lyx.org/"
  ${REG_UNINSTALL} "URLInfoAbout" "http://www.lyx.org/about/"
  ${REG_UNINSTALL} "Publisher" "LyX Team"
  ${REG_UNINSTALL} "HelpLink" "http://www.lyx.org/internet/mailing.php"  
  
SectionEnd

#--------------------------------
# Write LyX configuration file

Section -Configure

  # Windows specific configuration in lyxrc.dist

  Delete "$INSTDIR\Resources\lyxrc.dist"
  FileOpen $DistFile "$INSTDIR\Resources\lyxrc.dist" w

  # Path prefix

  StrCpy $PathPrefix "$INSTDIR\bin;$INSTDIR\python"
  
  ${If} $PathLaTeX != ""
    StrCpy $PathPrefix "$PathPrefix;$PathLaTeX"
  ${EndIf}
  ${If} $PathGhostscript != ""
    StrCpy $PathPrefix "$PathPrefix;$PathGhostscript"
  ${EndIf}
  ${If} $PathImageMagick != ""
    StrCpy $PathPrefix "$PathPrefix;$PathImageMagick"
  ${EndIf}

  FileWrite $DistFile '\path_prefix "$PathPrefix"$\r$\n'

  # Default screen fonts
  FileWrite $DistFile '\screen_font_roman "Times New Roman"$\r$\n'
  FileWrite $DistFile '\screen_font_sans "Arial"$\r$\n'
  FileWrite $DistFile '\screen_font_typewriter "Courier New"$\r$\n'
  FileWrite $DistFile '\preview_scale_factor 1.0$\r$\n' # Fit instant preview font size to screen fonts
  # PDF view helper
  FileWrite $DistFile '\format "pdf" "pdf" "PDF (ps2pdf)" "P" "pdfview" "" "document,vector"$\r$\n'  
  FileWrite $DistFile '\format "pdf2" "pdf" "PDF (pdflatex)" "F" "pdfview" "" "document,vector"$\r$\n'
  FileWrite $DistFile '\format "pdf3" "pdf" "PDF (dvipdfm)" "m" "pdfview" "" "document,vector"$\r$\n'

  FileClose $DistFile

  # Set language
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETTINGS} "Language" $LangISOCode

SectionEnd

#--------------------------------
# dvipost package

Var UpdateFNDBReturn

Section -dvipost

  # Install package in local root

  ${if} $PathLaTeXLocal != ""
    SetOutPath "$PathLaTeXLocal\tex\latex\dvipost"
    File "${FILES_DVIPOST_PKG}\dvipost.sty"
  ${endif}

  # Update file name database

  ${if} $PathLaTeX != ""
    nsExec::Exec '"$PathLaTeX\initexmf.exe" --update-fndb'
    Pop $UpdateFNDBReturn # Return value
  ${endif}

SectionEnd


#--------------------------------
# Postscript printer for metafile to EPS converter

Section -PSPrinter

  ${If} $MultiUser.Privileges != "Admin"
    ${OrIf} $MultiUser.Privileges != "Power"

    # Delete printer
    ExecWait '$PrinterConf /q /dl /n "Metafile to EPS Converter"'

    # Delete printer driver
    ExecWait '$PrinterConf /q /dd /m "Metafile to EPS Converter"'

    # Install printer and driver
    ExecWait '$PrinterConf /q /if /f "$INSTDIR\PSPrinter\metafile2eps.inf" /r "FILE:" /m "Metafile to EPS Converter"'

    # Restore DEVMODE with proper settings
    ExecWait '$PrinterConf /q /Sr /n "Metafile to EPS Converter" /a "$INSTDIR\PSPrinter\metafile2eps.dat" g'

  ${EndIf}

SectionEnd

#--------------------------------
# Desktop shortcut

Function CheckDesktopShortcut

  # Enable desktop icon creation when there is an icon already
  # Old shortcuts need to be updated
  
  ${If} ${FileExists} "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk"
    ${NSD_SetState} $mui.FinishPage.ShowReadme ${BST_CHECKED}
  ${EndIf}

FunctionEnd

Function CreateDesktopShortcut

  # Desktop icon creation is an option on the finish page

  CreateShortCut "$DESKTOP\${SHORTCUT}

FunctionEnd
