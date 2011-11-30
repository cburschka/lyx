/*

install.nsh

Installation of program files, dictionaries and external components

*/

#--------------------------------
# Program files

Var PythonCompileFile
Var PythonCompileReturn
Var DownloadResult

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
  
  SetOutPath "$INSTDIR\Resources"
  File /r "${FILES_DICT_THES}\dicts"
  File /r "${FILES_DICT_THES}\thes"
  
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
  !insertmacro FileListImageMagick File "${FILES_IMAGEMAGICK}\"
  !insertmacro FileListMSVC File "${FILES_MSVC}\"
  
  # Components of Ghostscript
  SetOutPath "$INSTDIR\ghostscript"
  !insertmacro FileListGhostscript File "${FILES_GHOSTSCRIPT}\"
  !insertmacro FileListMSVC File "${FILES_MSVC}\"
  
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
# Extenral components

Var PathAllUsers
Var PathCurrentUser

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
    
    # Updates the path environment variable of the instaler process to the latest system value
    ReadRegStr $PathAllUsers HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" Path
    ReadRegStr $PathCurrentUser HKCU "Environment" Path
    System::Call 'kernel32::SetEnvironmentVariableA(t, t) i("Path", "$PathAllUsers;$PathCurrentUser").'

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

Section -LaTeX ExternalLaTeX
  !insertmacro EXTERNAL LaTeX
SectionEnd

Function InitExternal

  # Get sizes of external component installers
  
  SectionGetSize ${ExternalLaTeX} $SizeLaTeX
  
  # Add download size
  
  !ifndef BUNDLESETUP_MIKTEX
    IntOp $SizeLaTeX $SizeLaTeX + ${SIZE_DOWNLOAD_LATEX}
  !endif

FunctionEnd
