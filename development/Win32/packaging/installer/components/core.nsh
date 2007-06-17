/*

LyX Core
Binaries, Resources, Python, Aspell

*/

#--------------------------------
#Sections

Section -Core SecCore

  # Install and register the core LyX files
  # Initializes the plug-ins dir ($PLUGINSDIR) if not already initialized.
  # $PLUGINSDIR is automatically deleted when the installer exits.
  InitPluginsDir
  
  # Binaries
  
  SetOutPath "$INSTDIR\bin"

  # launcher becomes lyx.exe while the real lyx.exe is renamed to lyxc.exe
  File "${FILES_LAUNCHER}\lyx.exe"
  File /oname=lyxc.exe "${FILES_LYX}\bin\lyx.exe"
  
  # The macros are defined in filelists.nsh
  # the parameters are COMMAND DIRECTORY that form command '${COMMAND} "${DIRECTORY}files"
  !insertmacro FileListLyXBin File "${FILES_LYX}\bin\"
  !insertmacro FileListQtBin File "${FILES_QT}\bin\"
  
  !insertmacro FileListDllMSVCBin File "${FILES_DEPS}\bin\"
  !insertmacro FileListMSVCBin File "${FILES_MSVC}\"
  !insertmacro FileListMSVCManifest File "..\"    

  !insertmacro FileListNetpbmBin File "${FILES_NETPBM}\"
  !insertmacro FileListDvipostBin File "${FILES_DVIPOST}\"
  !insertmacro FileListDTLBin File "${FILES_DTL}\"
  !insertmacro FileListPDFViewBin File "${FILES_PDFVIEW}\"
  !insertmacro FileListPDFToolsBin File "${FILES_PDFTOOLS}\"

  # Icons
  
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXIcons File "${FILES_ICONS}\"

  # Resources
  
  SetOutPath "$INSTDIR"
  # recursively copy all files under Resources
  File /r "${FILES_LYX}\Resources"
  
  #Components of Python
  
  SetOutPath "$INSTDIR\python"
  !insertmacro FileListPythonBin File "${FILES_PYTHON}\"
  SetOutPath "$INSTDIR\python"
  !insertmacro FileListPythonDll File "${FILES_WINDOWS}\System32\"
  !insertmacro FileListUnicodeDll File "${FILES_PYTHON}\DLLs\"
  SetOutPath "$INSTDIR\python\Lib"
  !insertmacro FileListPythonLib File "${FILES_PYTHON}\Lib\"
  SetOutPath "$INSTDIR\python\Lib\encodings"
  !insertmacro FileListPythonLibEncodings File "${FILES_PYTHON}\Lib\encodings\"
  
  # Aspell

  # copy installer to pluginsdir (a temp dir)
  File /oname=$PLUGINSDIR\AspellData.exe "${FILES_ASPELLDATA}\AspellData.exe"

  # silently install AspellData.exe (/S option)
  ${if} $CurrentUserInstall == ${TRUE}
    ExecWait '"$PLUGINSDIR\AspellData.exe" /S /CurrentUser'
  ${else}
    ExecWait '"$PLUGINSDIR\AspellData.exe" /S /AllUsers'
  ${endif}

  # remove the installer
  Delete "$PLUGINSDIR\AspellData.exe"

  # Aiksarus data
  
  SetOutPath "$INSTDIR\aiksaurus"
  !insertmacro FileListAiksaurusData File "${FILES_AIKSAURUS}\"
  
  # Helper DLLs for NSIS-based tools
  
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListNSISPluginsStandard File "${NSISDIR}\Plugins\"
  !insertmacro FileListNSISPlugins File "${FILES_NSISPLUGINS}\"
  
  # Create uninstaller
  WriteUninstaller "$INSTDIR\${SETUP_UNINSTALLER}"

SectionEnd
