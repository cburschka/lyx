/*

configure.nsh

Write registry information and configure LyX

*/

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
  SetOutPath "$INSTDIR\bin" # this is the folder in which the shortcut is executed
  # we must assure that the folder is not empty (happens on silent install and can accidentally happen)
  ${if} $StartmenuFolder == ""
   StrCpy $StartmenuFolder "${APP_DIR}"
  ${endif}
  CreateDirectory "$SMPROGRAMS\$StartmenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartmenuFolder\${APP_NAME}.lnk" "$INSTDIR\${APP_RUN}" "" "$INSTDIR\${APP_RUN}" "" "" "" "${APP_INFO}"
  # Link to www.lyx.org and to the Wiki
  WriteINIStr "$SMPROGRAMS\$StartmenuFolder\${APP_WEBPAGE_INFO}.url" "InternetShortcut" "URL" "${APP_WEBPAGE}"
  WriteINIStr "$SMPROGRAMS\$StartmenuFolder\${APP_WIKI_INFO}.url" "InternetShortcut" "URL" "${APP_WIKI}"
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
  WriteRegStr SHCTX ${APP_UNINST_KEY} "DisplayIcon" "$INSTDIR\bin\lyx,0"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "URLUpdateInfo" "http://www.lyx.org/"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "URLInfoAbout" "http://www.lyx.org/about/"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "Publisher" "LyX Team"
  WriteRegStr SHCTX ${APP_UNINST_KEY} "HelpLink" "http://www.lyx.org/internet/mailing.php"
  WriteRegDWORD SHCTX ${APP_UNINST_KEY} "NoModify" 0x00000001
  WriteRegDWORD SHCTX ${APP_UNINST_KEY} "NoRepair" 0x00000001
  WriteRegStr SHCTX ${APP_UNINST_KEY} "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
  
  # if we install over an older existing version, remove the old uninstaller information
  ${if} $OldVersionNumber < ${APP_SERIES_KEY}
   DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}$OldVersionNumber"
   # also delete in the case of an emergency release
   DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}$OldVersionNumber1"
  ${endif}
  
SectionEnd

#--------------------------------
# Write LyX configuration file

Section -Configure

  # Associate .lyx files with LyX for current user or all users

  ${if} $CreateFileAssociations == "true"
   WriteRegStr SHCTX "${APP_DIR_REGKEY}" "" "$INSTDIR\${APP_RUN}"
  ${endif}

  # Write information about file type
  #!define REG_FILETYPE 'WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}"'
  
  ${if} $CreateFileAssociations == "true"
   WriteRegStr SHCTX "${APP_DIR_REGKEY}" "OnlyWithLyX" "Yes${APP_SERIES_KEY}" # special entry to test if they were registered by this LyX version
   WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}" "" "${APP_NAME} Document"
   WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}\DefaultIcon" "" "$INSTDIR\${APP_RUN},0"
   WriteRegStr SHCTX "Software\Classes\${APP_REGNAME_DOC}\Shell\open\command" "" '"$INSTDIR\${APP_RUN}" "%1"'
   # we need to update also the automatically created entry about the lyx.exe
   # otherwise .lyx-files will could be opened with an older LyX version
   ReadRegStr $0 SHCTX "Software\Classes\Applications\${BIN_LYX}\shell\open\command" ""
   ${if} $0 != "" # if something was found
    WriteRegStr SHCTX "Software\Classes\Applications\${BIN_LYX}\shell\open\command" "" '"$INSTDIR\${APP_RUN}" "%1"'
   ${endif}
   # .lyx
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}" "Content Type" "${APP_MIME_TYPE}"
   # .lyx13
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}13" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}13" "Content Type" "${APP_MIME_TYPE}"
   # .lyx14
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}14" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}14" "Content Type" "${APP_MIME_TYPE}"
   # .lyx15
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}15" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}15" "Content Type" "${APP_MIME_TYPE}"
   # .lyx16
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}16" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}16" "Content Type" "${APP_MIME_TYPE}"
   # .lyx20
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}20" "" "${APP_REGNAME_DOC}"
   WriteRegStr SHCTX "Software\Classes\${APP_EXT}20" "Content Type" "${APP_MIME_TYPE}"
   # .lyx21 don't set this, because this is designed to be opened with LyX 2.1.x
  
   # Refresh shell
   ${RefreshShellIcons}
  ${endif}
  
  # Install standard lyxrc.dist file
  #SetOutPath "$INSTDIR\Resources"
  #File "${FILES_DEPS}\Resources\lyxrc.dist"

  # create the path prefix
  # $$ represents a literal $ in an NSIS string
  StrCpy $PathPrefix "$$LyXDir\bin;$$LyXDir\Python;$$LyXDir\Python\Lib;$$LyXDir\Perl\bin;$$LyXDir\imagemagick"
  
  ${if} $PathLaTeX != ""
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
  ${if} $GhostscriptPath != ""
   StrCpy $PathPrefix "$PathPrefix;$GhostscriptPath"
  ${endif}
  ${if} $SVGPath != ""
   StrCpy $PathPrefix "$PathPrefix;$SVGPath"
  ${endif}
  ${if} $GnumericPath != ""
   StrCpy $PathPrefix "$PathPrefix;$GnumericPath"
  ${endif}
  ${if} $PathBibTeXEditor != ""
    StrCpy $PathPrefix "$PathPrefix;$PathBibTeXEditor"
  ${EndIf}
  ${if} $PandocPath != ""
   StrCpy $PathPrefix "$PathPrefix;$PandocPath"
  ${endif}
  ${if} $LilyPondPath != ""
   StrCpy $PathPrefix "$PathPrefix;$LilyPondPath"
  ${endif}
  
  # Set the path prefix in lyxrc.dist
  ClearErrors
  Delete "$INSTDIR\Resources\lyxrc.dist"
  FileOpen $R1 "$INSTDIR\Resources\lyxrc.dist" w
  # set the format to the latest LyXRC format
  FileWrite $R1 'Format 19$\r$\n'
  # set some general things
  FileWrite $R1 '\screen_zoom 120$\r$\n'
  ${if} "$PathPrefix" != ""
   FileWrite $R1 '\path_prefix "$PathPrefix"$\r$\n'
  ${endif}
  
  # use pdfview for all types of PDF files
  FileWrite $R1 '\format "pdf5" "pdf" "PDF (LuaTeX)" "u" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		 \format "pdf4" "pdf" "PDF (XeTeX)" "X" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		 \format "pdf3" "pdf" "PDF (dvipdfm)" "m" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		 \format "pdf2" "pdf" "PDF (pdflatex)" "F" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n\
		 \format "pdf" "pdf" "PDF (ps2pdf)" "P" "pdfview" "" "document,vector,menu=export" "application/pdf"$\r$\n'
  
  # use Inkscape to edit PDF and EPS images
  ${if} $SVGPath != ""
   FileWrite $R1 '\format "pdf6" "pdf" "PDF (graphics)" "" "auto" "inkscape" "vector" "application/pdf"$\r$\n\
   				  \format "eps" "eps" "EPS" "" "auto" "inkscape" "vector" "image/x-eps"$\r$\n'
  ${endif}
  
  # if Inkscape is not available Imagemagick will be used to convert WMF/EMF files
  # We need to specify a resolution for the converter otherwise 1024 dpi are used and
  # eps2pdf takes ages. 300 dpi are a good compromise for speed and size.
  ${if} $SVGPath == ""
   FileWrite $R1 '\converter "wmf" "eps" "convert -density 300 $$$$i $$$$o" ""$\r$\n\
         \converter "emf" "eps" "convert -density 300 $$$$i $$$$o" ""$\r$\n'
  ${endif}
  
  # if LilyPondPath was found
  # We need to add these entris because python scripts can only be executed
  # if the full path is given.
  ${if} $LilyPondPath != ""
   FileWrite $R1 '\format "lilypond-book" "lytex" "LilyPond book (LaTeX)" "" "" "auto" "document,menu=export" ""$\r$\n\
		  \converter "lilypond-book" "pdflatex" "python \"$LilyPondPath\\lilypond-book.py\" --safe --pdf --latex-program=pdflatex --lily-output-dir=ly-pdf $$$$i" ""$\r$\n\
		  \converter "lilypond-book" "xetex" "python \"$LilyPondPath\\lilypond-book.py\" --safe --pdf --latex-program=xelatex --lily-output-dir=ly-pdf $$$$i" ""$\r$\n\
		  \converter "lilypond-book" "luatex" "python \"$LilyPondPath\\lilypond-book.py\" --safe --pdf --latex-program=lualatex --lily-output-dir=ly-pdf $$$$i" ""$\r$\n\
		  \converter "lilypond-book" "latex" "python \"$LilyPondPath\\lilypond-book.py\" --safe --lily-output-dir=ly-eps $$$$i" ""$\r$\n'
  ${endif}
  FileClose $R1
  IfErrors 0 +2
   MessageBox MB_OK|MB_ICONEXCLAMATION "$(ModifyingConfigureFailed)" /SD IDOK
  ClearErrors
  
  # for texindy the path to the perl.exe must unfortunately be in Windows' PATH variable
  ${if} $MultiUser.Privileges != "Admin"
  ${andif} $MultiUser.Privileges != "Power"
   # call the non-admin version
   ${EnvVarUpdate} $0 "PATH" "A" "HKCU" "$INSTDIR\Perl\bin"
  ${else}
   ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\Perl\bin"
  ${endif}

SectionEnd

#--------------------------------
# Run the LyX configure.py script, so MiKTeX can download its packages

Var ConfigureReturn

Section -ConfigureScript

  SetOutPath "$INSTDIR\Resources"
  
  # ask to update MiKTeX
  ${if} $LaTeXInstalled == "MiKTeX"
   Call UpdateMiKTeX # function from latex.nsh
   # install all necessary packages at once because this is much faster then to install the packages one by one
   DetailPrint $(TEXT_CONFIGURE_LYX)
   ${if} $MultiUser.Privileges != "Admin"
   ${andif} $MultiUser.Privileges != "Power"
    # call the non-admin version
    # at first we need to synchronize the package database
    nsExec::ExecToLog '"$PathLaTeX\mpm.exe" "--verbose" "--update-db"'
    nsExec::ExecToLog '"$PathLaTeX\mpm.exe" "--verbose" "--install-some=$INSTDIR\Resources\Packages.txt"'
   ${else}
    ${if} $MiKTeXUser != "HKCU" # call the admin version
     nsExec::ExecToLog '"$PathLaTeX\mpm.exe" "--admin" "--verbose" "--update-db"'
     nsExec::ExecToLog '"$PathLaTeX\mpm.exe" "--admin" "--verbose" "--install-some=$INSTDIR\Resources\Packages.txt"'
    ${else}
     nsExec::ExecToLog '"$PathLaTeX\mpm.exe" "--verbose" "--update-db"'
     nsExec::ExecToLog '"$PathLaTeX\mpm.exe" "--verbose" "--install-some=$INSTDIR\Resources\Packages.txt"'
    ${endif}
   ${endif}
  ${endif}
  
  DetailPrint $(TEXT_CONFIGURE_LYX)
  nsExec::ExecToLog '"$INSTDIR\Python\python.exe" "$INSTDIR\Resources\configure.py"'
  # $ConfigureReturn is "0" if successful, otherwise "1"
  Pop $ConfigureReturn # Return value

SectionEnd

#--------------------------------
# Desktop shortcut

Function StartLyX

  # run LyX in a command line window to give the users feedback about
  # the time consuming LaTeX package installation
  
  #Exec 'cmd /K " "$INSTDIR\bin\lyx.exe""'
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

