/*

configure.nsh

Write registry information and configure LyX

*/

#Var DistFile
Var PathPrefix

#!define SHORTCUT '${APP_NAME} ${APP_SERIES_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"'

#--------------------------------
# Registry information

Section -InstallData

  # Registry information
  WriteRegStr SHCTX ${APP_REGKEY} "" $INSTDIR
  WriteRegStr SHCTX ${APP_REGKEY} "Version" "${APP_VERSION_NUMBER}"
  WriteRegStr SHCTX ${APP_REGKEY_SETUP} "LaTeX Path" $PathLaTeX
  
  # Start Menu shortcut
  # There is only one shortcut to the application, so it should be in the main group
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${APP_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"
  # create desktop icon
  ${if} $CreateDesktopIcon == "true"
   SetOutPath "$INSTDIR\bin"
   CreateShortCut "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"
  ${endif}
  
  # Uninstaller information
  ${If} $MultiUser.InstallMode == "CurrentUser"
    WriteRegStr SHCTX ${APP_UNINST_KEY} "DisplayName" "${APP_NAME} ${APP_VERSION} $(TEXT_INSTALL_CURRENTUSER)"
  ${Else}
    WriteRegStr SHCTX ${APP_UNINST_KEY} "DisplayName" "${APP_NAME} ${APP_VERSION}"
  ${EndIf}
  
  WriteRegStr SHCTX ${APP_UNINST_KEY} "UninstallString" '"$INSTDIR\${SETUP_UNINSTALLER}"'
  WriteRegStr SHCTX ${APP_UNINST_KEY} "DisplayVersion" "${APP_VERSION}"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "DisplayIcon" "$INSTDIR\bin\LyXLauncher,0"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "URLUpdateInfo" "http://www.lyx.org/"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "URLInfoAbout" "http://www.lyx.org/about/"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "Publisher" "LyX Team"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "HelpLink" "http://www.lyx.org/internet/mailing.php"
  WriteRegDWORD SHCTX ${APP_UNINST_KEY} "NoModify" 0x00000001
  WriteRegDWORD SHCTX ${APP_UNINST_KEY} "NoRepair" 0x00000001
  WriteRegStr SHCTX ${APP_UNINST_KEY} "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
  
SectionEnd

#--------------------------------
# Write LyX configuration file

Section -Configure

  # Associate .lyx files with LyX for current user of all users

  ${if} $CreateFileAssociations == "true"
   WriteRegStr HKLM "${APP_DIR_REGKEY}" "" "$INSTDIR\${APP_RUN}"
  ${endif}

  # Write information about file type
  #!define REG_FILETYPE 'WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}"'
  
  ${if} $CreateFileAssociations == "true"
   WriteRegStr SHCTX "${APP_DIR_REGKEY}" "OnlyWithLyX" "Yes${APP_SERIES_KEY}" # special entry to test if they were registered by this LyX version
   WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}" "" "${APP_NAME} Document"
   WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}\DefaultIcon" "" "$INSTDIR\${APP_RUN},0"
   WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}\Shell\open\command" "" '"$INSTDIR\${APP_RUN}" "%1"'
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}" "Content Type" "${APP_MIME_TYPE}"
   # .lyx13
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}13" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}13" "Content Type" "${APP_MIME_TYPE}"
   # .lyx14
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}14" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}14" "Content Type" "${APP_MIME_TYPE}"
   # .lyx15 don't set this, because this is designed to be opened with LyX 1.5.x
   #WriteRegStr SHCTX "Software\Classes\${APP_EXT}15" "" "${PRODUCT_REGNAME}"
   #WriteRegStr SHCTX "Software\Classes\${APP_EXT}15" "Content Type" "${PRODUCT_MIME_TYPE}"
   # .lyx16 don't set this, because this is designed to be opened with LyX 1.6.x
   #WriteRegStr SHCTX "Software\Classes\${APP_EXT}16" "" "${PRODUCT_REGNAME}"
   #WriteRegStr SHCTX "Software\Classes\${APP_EXT}16" "Content Type" "${PRODUCT_MIME_TYPE}"
  
   # Refresh shell
   ${RefreshShellIcons}
  ${endif}
  
  # Set path prefix in lyxrc.dist

  # Install standard lyxrc.dist file
  #SetOutPath "$INSTDIR\Resources"
  #File "${FILES_DEPS}\Resources\lyxrc.dist"

  # create the path prefix
  # $$ represents a literal $ in an NSIS string
  StrCpy $PathPrefix "$$LyXDir\bin;$$LyXDir\python;$$LyXDir\imagemagick;$$LyXDir\ghostscript"
  
  ${If} $PathLaTeX != ""
    StrCpy $PathPrefix "$PathPrefix;$PathLaTeX"
  ${EndIf}
  ${if} $PSVPath != ""
    StrCpy $PathPrefix "$PathPrefix;$PSVPath"
  ${endif}
  ${if} $EditorPath != ""
    StrCpy $PathPrefix "$PathPrefix;$EditorPath"
  ${endif}
  ${if} $ImageEditorPath != ""
    StrCpy $PathPrefix "$PathPrefix;$ImageEditorPath"
  ${endif}
  ${if} $SVGPath != ""
   StrCpy $PathPrefix "$PathPrefix;$SVGPath"
  ${endif}
  ${if} $WMFPath != ""
   StrCpy $PathPrefix "$PathPrefix;$WMFPath"
  ${endif}
  ${If} $PathBibTeXEditor != ""
    StrCpy $PathPrefix "$PathPrefix;$PathBibTeXEditor"
  ${EndIf}  

  # Set the path prefix in lyxrc.dist
  ClearErrors
  Delete "$INSTDIR\Resources\lyxrc.dist"
  FileOpen $R1 "$INSTDIR\Resources\lyxrc.dist" w
  # set some general things
  FileWrite $R1 '\screen_zoom "120"$\r$\n\
                 \accept_compound true$\r$\n'
  ${if} "$PathPrefix" != ""
   FileWrite $R1 '\path_prefix "$PathPrefix"$\r$\n'
  ${endif}
  # if Acrobat or Adobe Reader is used
  ${if} $Acrobat == "Yes" # used for Acrobat / Adobe Reader
   FileWrite $R1 '\format "pdf3" "pdf" "PDF (dvipdfm)" "m" "pdfview" "" "document,vector"$\r$\n\
   		  \format "pdf2" "pdf" "PDF (pdflatex)" "F" "pdfview" "" "document,vector"$\r$\n\
		  \format "pdf" "pdf" "PDF (ps2pdf)" "P" "pdfview" "" "document,vector"$\r$\n'
  ${endif}
  # if a SVG to PDF converter ws found (e.g. Inkscape)
  ${if} $SVGPath != ""
   FileWrite $R1 '\format "svg" "svg" "SVG" "" "inkscape --file=$$$$i" "inkscape --file=$$$$i" "vector"$\r$\n\
   		  \converter "svg" "png" "inkscape --without-gui --file=$$$$i --export-png=$$$$o" ""$\r$\n\
		  \converter "svg" "pdf" "inkscape --file=$$$$p/$$$$i --export-area-drawing --without-gui --export-pdf=$$$$p/$$$$o" ""$\r$\n\
		  \converter "svg" "eps" "inkscape --file=$$$$p/$$$$i --export-area-drawing --without-gui --export-eps=$$$$p/$$$$o" ""'
  ${endif}
  FileClose $R1
  IfErrors 0 +2
   MessageBox MB_OK|MB_ICONEXCLAMATION "$(ModifyingConfigureFailed)"
  ClearErrors

SectionEnd

#--------------------------------
# LaTeX files

/*Var UpdateFNDBReturn

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
  
SectionEnd*/

#--------------------------------
# Postscript printer for metafile to EPS converter

Section -PSPrinter

  ${If} $MultiUser.Privileges == "Admin"
    ${OrIf} $MultiUser.Privileges == "Power"

    # Delete printer
    ExecWait '$PrinterConf /q /dl /n "Metafile to EPS Converter"'
    # Install printer and driver
    ExecWait '$PrinterConf /if /f "$WINDIR\inf\ntprint.inf" /b "Metafile to EPS Converter" /r "FILE:" /m "MS Publisher Imagesetter"'
    # Restore DEVMODE with proper settings
    ExecWait '$PrinterConf /q /Sr /n "Metafile to EPS Converter" /a "$INSTDIR\bin\metafile2eps.dat" g'

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

  # ask to update MiKTeX
  ${if} $LaTeXInstalled == "MiKTeX"
   Call UpdateMiKTeX # function from latex.nsh
  ${endif}

SectionEnd

#--------------------------------
# Desktop shortcut

Function StartLyX

  # Enable desktop icon creation when there is an icon already
  # Old shortcuts need to be updated
  
  Exec "$INSTDIR\${APP_RUN}"

FunctionEnd

/*Function CheckDesktopShortcut

  # Enable desktop icon creation when there is an icon already
  # Old shortcuts need to be updated
  
  ${If} ${FileExists} "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk"
    ${NSD_SetState} $mui.FinishPage.ShowReadme ${BST_CHECKED}
  ${EndIf}

FunctionEnd

Function CreateDesktopShortcut

  # Desktop icon creation is an option on the finish page
  SetOutPath "$INSTDIR\bin"
  CreateShortCut "$DESKTOP\${APP_NAME} ${APP_SERIES_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"

FunctionEnd*/

