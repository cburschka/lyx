/*

install.nsh

Installation of program files, dictionaries and external components

*/

#--------------------------------
# Program files

Var PythonCompileFile
Var PythonCompileReturn
#Var DownloadResult

Section -ProgramFiles SecProgramFiles

  # Install and register the core LyX files
  
  # The macros are defined in filelists.nsh
  # the parameters are COMMAND DIRECTORY that form command '${COMMAND} "${DIRECTORY}files"  
  
  # Initializes the plug-ins dir ($PLUGINSDIR) if not already initialized.
  # $PLUGINSDIR is automatically deleted when the installer exits.
  InitPluginsDir
  
  # Binaries
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXBin File "${FILES_LYX}\bin\"
  !insertmacro FileListQtBin File "${FILES_QT}\bin\"
  !insertmacro FileListQtImageformats File "${FILES_QT}\plugins\imageformats\"
  !insertmacro FileListDll File "${FILES_DEPS}\bin\"
  !insertmacro FileListMSVC File "${FILES_MSVC}\"
  !insertmacro FileListNetpbmBin File "${FILES_NETPBM}\"
  !insertmacro FileListDTLBin File "${FILES_DTL}\"
  !insertmacro FileListDvipostBin File "${FILES_DVIPOST}\"
  !insertmacro FileListPDFViewBin File "${FILES_PDFVIEW}\"
  !insertmacro FileListPDFToolsBin File "${FILES_PDFTOOLS}\"
  !insertmacro FileListMetaFile2EPS File "${FILES_METAFILE2EPS}\"
  
  # Resources
  SetOutPath "$INSTDIR"
  # recursively copy all files under Resources
  File /r "${FILES_LYX}\Resources"
  
  !if ${SETUPTYPE} == BUNDLE
   
   # extract the Jabref and MiKTeX installer
   File /r "${FILES_LYX}\external"

   # install MiKTeX if not already installed
   Call InstallMiKTeX # function from LaTeX.nsh
   
  !endif # end if BUNDLE
  
  # Python
  SetOutPath "$INSTDIR"
  # recursively copy all files under Python
  File /r "${FILES_PYTHON}"
  
  # Compile all Pyton files to byte-code
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
  WriteRegStr SHCTX "SOFTWARE\Classes\Applications" "AutoRun" "$INSTDIR\imagemagick\convert.exe $$"
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
  
  # Components of Ghostscript
  ${if} $GhostscriptPath == ""
   SetOutPath "$INSTDIR\ghostscript"
   File /r "${FILES_GHOSTSCRIPT}\"
   !insertmacro FileListMSVC File "${FILES_MSVC}\"
   StrCpy $GhostscriptPath "$INSTDIR\ghostscript\bin"
  ${endif}
  
  !if ${SETUPTYPE} == BUNDLE
   
   # install JabRef if not already installed and the user selected it
   # if no BibTeX editor is installed
   ${if} $PathBibTeXEditor == ""
    ${if} $InstallJabRef == "true"
     # launch installer
     MessageBox MB_OK|MB_ICONINFORMATION "$(JabRefInfo)"
     ExecWait "$INSTDIR\${JabRefInstall}"
     # test if JabRef is now installed
     StrCpy $PathBibTeXEditor ""
     ReadRegStr $PathBibTeXEditor HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "UninstallString"    
     ${if} $PathBibTeXEditor == ""
      MessageBox MB_OK|MB_ICONEXCLAMATION "$(JabRefError)"
     ${else}
      WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX" "Yes${APP_SERIES_KEY}" # special entry to tell the uninstaller that it was installed with LyX
     ${endif}
    ${endif}
   ${endif}
  !endif # end if BUNDLE
  
   # install eLyXer
   SetOutPath "$INSTDIR\Python\Lib"
   !insertmacro FileListeLyXer File "${FILES_ELYXER}\"
   
   # install unoconv
   SetOutPath "$INSTDIR\Python\Lib"
   !insertmacro FileListUnoConv File "${FILES_UNOCONV}\"

  # install the LaTeX class files that are delivered with LyX
  # and enable MiKTeX's automatic package installation
  ${if} $LaTeXInstalled == "MiKTeX"
   Call ConfigureMiKTeX # Function from LaTeX.nsh
  ${endif}
  
  # download dictionaries and thesaurus
  ${if} $DictCodes != ""
   Call InstallHunspellDictionary # Function from Thesaurus.nsh
  ${endif}
  ${if} $ThesCodes != ""
   Call InstallThesaurusDictionary # Function from Thesaurus.nsh
  ${endif}
  # finally delete the list of dictionaries and mirrors
  Delete "$INSTDIR\Resources\DictionaryMirrors.txt"
  Delete "$INSTDIR\Resources\ThesaurusDictionaryNames.txt"
  Delete "$INSTDIR\Resources\HunspellDictionaryNames.txt"
  
  # Create uninstaller
  WriteUninstaller "$INSTDIR\${SETUP_UNINSTALLER}"

SectionEnd

#--------------------------------
# Support code for file downloads

!macro DOWNLOAD_FILE RET ID FILENAME APPEND

  # Downloads a file
  
  # RET = Return value (OK if succesful)
  # ID = Name of the download in settings.nsh
  # FILENAME = Location to store file
  # APPEND = Filename to append to server location in settings.nsh

  # Try first time
  NSISdl::download "${DOWNLOAD_${ID}}${APPEND}" "$PLUGINSDIR\${FILENAME}"
  Pop ${RET} # Return value (OK if succesful)

  ${If} ${RET} != "success"
    ${AndIf} ${RET} != "cancel"
    # Download failed, try once again before giving up
    # (usally we get a different mirror)
    NSISdl::download "${DOWNLOAD_${ID}}${APPEND}" "$PLUGINSDIR\${FILENAME}"
    Pop ${RET}
  ${EndIf}

!macroend

#--------------------------------
# External components

#Var PathAllUsers
#Var PathCurrentUser

!macro EXTERNAL COMPONENT

  # Download/Install the component
  
  ${If} $Setup${COMPONENT} == ${TRUE}
  
    StrCpy $Path${COMPONENT} "" ;A new one will be installed
  
    !ifndef BUNDLESETUP_${COMPONENT}
      !insertmacro EXTERNAL_DOWNLOAD ${COMPONENT}
    !else
      !insertmacro EXTERNAL_INSTALL ${COMPONENT}
    !endif
    
  ${EndIf}

!macroend

!macro EXTERNAL_RUNINSTALLER COMPONENT

  # Run the installer application of the component that does the actual installation.

  install_${COMPONENT}:
      
    ExecWait '"$PLUGINSDIR\${COMPONENT}Setup.exe"'
    
    # Updates the path environment variable of the installer process to the latest system value
#    ReadRegStr $PathAllUsers HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" Path
#    ReadRegStr $PathCurrentUser HKCU "Environment" Path
#    System::Call 'kernel32::SetEnvironmentVariableA(t, t) i("Path", "$PathAllUsers;$PathCurrentUser").'

    Call Search${COMPONENT}
    
    ${If} $Path${COMPONENT} == ""  
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(TEXT_NOTINSTALLED_${COMPONENT}) IDYES install_${COMPONENT}
    ${EndIf}
      
    Delete "$PLUGINSDIR\${COMPONENT}Setup.exe"
     
!macroend

!macro EXTERNAL_DOWNLOAD COMPONENT

  download_${COMPONENT}:

    !insertmacro DOWNLOAD_FILE $DownloadResult "${COMPONENT}" "${COMPONENT}Setup.exe" ""
 
    ${If} $DownloadResult != "success"
      ${AndIf} $DownloadResult != "cancel"
      # Download failed after trying twice - ask user
      MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_${COMPONENT}) ($DownloadResult)" IDYES download_${COMPONENT}
      Goto noinstall_${COMPONENT}
    ${EndIf}
      
    !insertmacro EXTERNAL_RUNINSTALLER ${COMPONENT}
      
  noinstall_${COMPONENT}:

!macroend

!macro EXTERNAL_INSTALL COMPONENT

  # Extract
  File /oname=$PLUGINSDIR\${COMPONENT}Setup.exe ${FILES_BUNDLE}\${INSTALL_${COMPONENT}}
    
  !insertmacro EXTERNAL_RUNINSTALLER ${COMPONENT}
    
!macroend


# Sections for external components

#Section -LaTeX ExternalLaTeX
#  !insertmacro EXTERNAL LaTeX
#SectionEnd

/*Function InitExternal

  # Get sizes of external component installers
  
  #SectionGetSize ${ExternalLaTeX} $SizeLaTeX
  
  # Add download size
  
  !ifndef BUNDLESETUP_MIKTEX
    IntOp $SizeLaTeX $SizeLaTeX + ${SIZE_DOWNLOAD_LATEX}
  !endif

FunctionEnd*/
