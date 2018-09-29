/*

install.nsh

Installation of program files, dictionaries and external components

*/

#--------------------------------
# Program files

Var PythonCompileFile
Var PythonCompileReturn

Section -ProgramFiles SecProgramFiles

  # if the $INSTDIR does not contain "LyX" we must add a subfolder to avoid that LyX will e.g.
  # be installed directly to C:\programs - the uninstaller will then delete the whole
  # C:\programs directory
  StrCpy $String $INSTDIR
  StrCpy $Search "LyX"
  Call StrPoint # function from LyXUtils.nsh
  ${if} $Pointer == "-1"
   StrCpy $INSTDIR "$INSTDIR\${APP_DIR}"
  ${endif}

 # abort the installation if no LaTeX was found but should be used
 ${if} $PathLaTeX == ""
 ${andif} $State == "0"
  SetOutPath $TEMP # to be able to delete the $INSTDIR
  RMDir /r $INSTDIR
  Abort
 ${endif}

  # Install and register the core LyX files
  
  # The macros are defined in filelists.nsh
  # the parameters are COMMAND DIRECTORY that form command '${COMMAND} "${DIRECTORY}files"  
  
  # Initializes the plug-ins dir ($PLUGINSDIR) if not already initialized.
  # $PLUGINSDIR is automatically deleted when the installer exits.
  InitPluginsDir
  
  # Binaries
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXBin File "${FILES_LYX}\bin\"
  # The 2.3.0 and early 2.3.1 installers used a version suffix
  # We need to remove the old binaries if we're installing into the same directory
  Delete "$INSTDIR\bin\LyX2.3.exe"
  Delete "$INSTDIR\bin\tex2lyx2.3.exe"
  !insertmacro FileListQtBin File "${FILES_QT}\bin\"
  !insertmacro FileListMinGW File "${FILES_LYX}\bin\"
  !insertmacro FileListNetpbmBin File "${FILES_NETPBM}\"
  !insertmacro FileListDTLBin File "${FILES_DTL}\"
  !insertmacro FileListRsvg File "${FILES_RSVG}\"
  !insertmacro FileListDvipostBin File "${FILES_DVIPOST}\"
  !insertmacro FileListPDFViewBin File "${FILES_PDFVIEW}\"
  
  # Qt plugin DLLs
  SetOutPath "$INSTDIR\bin\imageformats"
  !insertmacro FileListQtImageformats File "${FILES_QT}\bin\imageformats\"
  SetOutPath "$INSTDIR\bin\iconengines"
  !insertmacro FileListQtIconengines File "${FILES_QT}\bin\iconengines\"
  SetOutPath "$INSTDIR\bin\platforms"
  !insertmacro FileListQtPlatforms File "${FILES_QT}\bin\platforms\"
  SetOutPath "$INSTDIR\bin\styles"
  !insertmacro FileListQtStyles File "${FILES_QT}\bin\styles\"
  
  # Resources
  SetOutPath "$INSTDIR"
  # recursively copy all files under Resources
  File /r "${FILES_LYX}\Resources"
  File /r "${FILES_DEPS}\Resources"
  
  # Python
  SetOutPath "$INSTDIR"
  # recursively copy all files under Python
  File /r "${FILES_PYTHON}"
#FIXME We probably should not do this, as dicussed on the list.
 # register .py files if necessary
  ReadRegStr $0 SHCTX "Software\Classes\Python.File\shell\open\command" ""
  ${if} $0 == "" # if nothing was found
   WriteRegStr SHCTX "Software\Classes\Python.File\shell\open\command" "" '"$INSTDIR\Python\python.exe" "%1" %*'
   WriteRegStr SHCTX "Software\Classes\Python.File\DefaultIcon" "" "$INSTDIR\Python\DLLs\py.ico"
   WriteRegStr SHCTX "Software\Classes\.py" "" "Python.File"
   WriteRegStr SHCTX "Software\Classes\Python.File" "OnlyWithLyX" "Yes${APP_SERIES_KEY}" # special entry to test if they were registered by this LyX version
  ${endif}
  
  # Compile all Python files to byte-code
  # The user using the scripts may not have write access
  FileOpen $PythonCompileFile "$INSTDIR\compilepy.py" w
  FileWrite $PythonCompileFile "import compileall$\r$\n"
  FileWrite $PythonCompileFile "compileall.compile_dir('$INSTDIR\python\Lib')$\r$\n"
  FileWrite $PythonCompileFile "compileall.compile_dir('$INSTDIR\Resources')$\r$\n"
  FileClose $PythonCompileFile
  DetailPrint $(TEXT_CONFIGURE_PYTHON)
  nsExec::ExecToLog '"$INSTDIR\python\python.exe" "$INSTDIR\compilepy.py"'
  Pop $PythonCompileReturn # Return value
  Delete "$INSTDIR\compilepy.py"
  
  # Components of ImageMagick
  SetOutPath "$INSTDIR\imagemagick"
  File /r "${FILES_IMAGEMAGICK}\"
  !insertmacro FileListMSVC File "${FILES_MSVC}\"
  # register ImageMagick
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "BinPath" "$INSTDIR\imagemagick"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "CoderModulesPath" "$INSTDIR\imagemagick\modules\coders"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "ConfigurePath" "$INSTDIR\imagemagick"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "FilterModulesPath" "$INSTDIR\imagemagick\modules\filters"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "LibPath" "$INSTDIR\imagemagick"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\Current" "BinPath" "$INSTDIR\imagemagick"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\Current" "CoderModulesPath" "$INSTDIR\imagemagick\modules\coders"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\Current" "ConfigurePath" "$INSTDIR\imagemagick"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\Current" "FilterModulesPath" "$INSTDIR\imagemagick\modules\filters"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\Current" "LibPath" "$INSTDIR\imagemagick"
  WriteRegDWORD SHCTX "SOFTWARE\ImageMagick\Current" "QuantumDepth" 0x00000010
  WriteRegStr SHCTX "SOFTWARE\ImageMagick\Current" "Version" "${ImageMagickVersion}"
  WriteRegStr SHCTX "SOFTWARE\ImageMagick" "OnlyWithLyX" "Yes${APP_SERIES_KEY}"
  
  # Components of Ghostscript
  ${if} $GhostscriptPath == ""
   SetOutPath "$INSTDIR\ghostscript"
   File /r "${FILES_GHOSTSCRIPT}\"
   !insertmacro FileListMSVC File "${FILES_MSVC}\"
   StrCpy $GhostscriptPath "$INSTDIR\ghostscript\bin"
  ${endif}
  
  # install unoconv
  SetOutPath "$INSTDIR\Python\Lib"
  !insertmacro FileListUnoConv File "${FILES_UNOCONV}\"

  # install the LaTeX class files that are delivered with LyX to MiKTeX
  ${if} $LaTeXInstalled == "MiKTeX"
   Call ConfigureMiKTeX # Function from LaTeX.nsh
  ${endif}
  # install the LaTeX class files that are delivered with LyX to TeXLive
  ${if} $LaTeXInstalled == "TeXLive"
   Call ConfigureTeXLive # Function from LaTeX.nsh
  ${endif}
  
  # download dictionaries and thesaurus
  ${if} $DictCodes != ""
   Call InstallHunspellDictionaries # Function from dictionaries.nsh
  ${endif}
  ${if} $ThesCodes != ""
   Call InstallThesaurusDictionaries # Function from dictionaries.nsh
  ${endif}
  # finally delete the list of mirrors
  Delete "$INSTDIR\Resources\DictionaryMirrors.txt"
  
  # Create uninstaller
  WriteUninstaller "$INSTDIR\${SETUP_UNINSTALLER}"

SectionEnd
