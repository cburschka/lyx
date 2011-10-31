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
  ${REG_FILETYPE}\DefaultIcon" "" "$INSTDIR\${APP_RUN},0"
  ${REG_FILETYPE}\Shell\open\command" "" '"$INSTDIR\${APP_RUN}" "%1"'
  
  !define REG_FILEEXT 'WriteRegStr SHELL_CONTEXT "Software\Classes\${APP_EXT}"'
  
  ${REG_FILEEXT} "" "${APP_REGNAME_DOC}"
  ${REG_FILEEXT} "Content Type" "${APP_MIME_TYPE}"  
  
  # Refresh shell
  ${RefreshShellIcons}

SectionEnd

#--------------------------------
# Registry information

Section -InstallData

  # Registry information
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY} "" $INSTDIR
  WriteRegStr SHELL_CONTEXT ${APP_REGKEY} "Version" "${APP_VERSION_NUMBER}"

  WriteRegStr SHELL_CONTEXT ${APP_REGKEY_SETUP} "LaTeX Path" $PathLaTeX

  # Start Menu shortcut
  # There is only one shortcut to the application, so it should be in the main group
  SetOutPath "$INSTDIR\bin"
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
  ${REG_UNINSTALL} "DisplayIcon" "$INSTDIR\${APP_RUN},0"
  ${REG_UNINSTALL} "URLUpdateInfo" "http://www.lyx.org/"
  ${REG_UNINSTALL} "URLInfoAbout" "http://www.lyx.org/about/"
  ${REG_UNINSTALL} "Publisher" "LyX Team"
  ${REG_UNINSTALL} "HelpLink" "http://www.lyx.org/internet/mailing.php"  
  
SectionEnd

#--------------------------------
# Write LyX configuration file

Section -Configure

  # Set path prefix in lyxrc.dist

  # Install standard lyxrc.dist file
  SetOutPath "$INSTDIR\Resources"
  File "${FILES_DEPS}\Resources\lyxrc.dist"

  # Append path prefix
  FileOpen $DistFile "$INSTDIR\Resources\lyxrc.dist" a
  FileSeek $DistFile 0 END

  # $$ represents a literal $ in an NSIS string
  StrCpy $PathPrefix "$$LyXDir\bin;$$LyXDir\python;$$LyXDir\imagemagick;$$LyXDir\ghostscript"
  
  ${If} $PathLaTeX != ""
    StrCpy $PathPrefix "$PathPrefix;$PathLaTeX"
  ${EndIf}
  ${If} $PathBibTeXEditor != ""
    StrCpy $PathPrefix "$PathPrefix;$PathBibTeXEditor"
  ${EndIf}  

  FileWrite $DistFile '\path_prefix "$PathPrefix"$\r$\n'

  FileClose $DistFile

SectionEnd

#--------------------------------
# LaTeX files

Var UpdateFNDBReturn

Section -LaTeXFiles

  # Install files in local root

  ${If} $PathLaTeXLocal != ""
    # dvipost
    SetOutPath "$PathLaTeXLocal\tex\latex\dvipost"
    File "${FILES_DVIPOST_PKG}\dvipost.sty"
    # LyX files in Resources\tex
    SetOutPath "$PathLaTeXLocal\tex\latex\lyx"
    CopyFiles /SILENT "$INSTDIR\Resources\tex\*.*" "$PathLaTeXLocal\tex\latex\lyx"
  ${EndIf}

  # Update file name database

  ${If} $PathLaTeX != ""
    DetailPrint $(TEXT_CONFIGURE_MIKTEXFNDB)
    nsExec::ExecToLog '"$PathLaTeX\initexmf.exe" --update-fndb'
    Pop $UpdateFNDBReturn # Return value
  ${EndIf}

SectionEnd

#--------------------------------
# Postscript printer for metafile to EPS converter

Section -PSPrinter

  ${If} $MultiUser.Privileges == "Admin"
    ${OrIf} $MultiUser.Privileges == "Power"

    # Delete printer
    ExecWait '$PrinterConf /q /dl /n "Metafile to EPS Converter"'
    # Install printer and driver
    ExecWait '$PrinterConf /if /f "$WINDIR\inf\ntprint.inf" /b "Metafile to EPS Converter" /r "FILE:" /m "MS Publisher Imagesetter"'

  ${EndIf}

SectionEnd

#--------------------------------
# Run the LyX configure.py script, so MiKTeX can download its packages

Var ConfigureReturn

Section -ConfigureScript

  SetOutPath "$INSTDIR\Resources"
  DetailPrint $(TEXT_CONFIGURE_LYX)
  nsExec::ExecToLog '"$INSTDIR\python\python.exe" "$INSTDIR\Resources\configure.py"'
  Pop $ConfigureReturn # Return value

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
  SetOutPath "$INSTDIR\bin"
  CreateShortCut "$DESKTOP\${SHORTCUT}

FunctionEnd
