/*

install.nsh

Installation of program files, dictionaries and external components

*/

#--------------------------------
# Program files

Section -ProgramFiles SecProgramFiles

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
  !insertmacro FileListDTLBin File "${FILES_DTL}\"
  !insertmacro FileListDvipostBin File "${FILES_DVIPOST}\"
  !insertmacro FileListPDFViewBin File "${FILES_PDFVIEW}\"
  !insertmacro FileListPDFToolsBin File "${FILES_PDFTOOLS}\"
  
  # Resources
  
  SetOutPath "$INSTDIR"
  # recursively copy all files under Resources
  File /r "${FILES_LYX}\Resources"
  
  # Components of Python
  
  SetOutPath "$INSTDIR\python"
  !insertmacro FileListPythonBin File "${FILES_PYTHON}\"
  !insertmacro FileListPythonDll File "$%SystemRoot%\System32\"
  !insertmacro FileListUnicodeDll File "${FILES_PYTHON}\DLLs\"
  SetOutPath "$INSTDIR\python\Lib"
  !insertmacro FileListPythonLib File "${FILES_PYTHON}\Lib\"
  SetOutPath "$INSTDIR\python\Lib\encodings"
  !insertmacro FileListPythonLibEncodings File "${FILES_PYTHON}\Lib\encodings\"
  
  # Aspell

  # Copy installer to pluginsdir (a temp dir)
  File /oname=$PLUGINSDIR\AspellData.exe "${FILES_ASPELLDATA}\AspellData.exe"

  # Silently install AspellData.exe (/S option)
  ${If} $MultiUser.InstallMode == "CurrentUser"
    ExecWait '"$PLUGINSDIR\AspellData.exe" /S /CurrentUser'
  ${Else}
    ExecWait '"$PLUGINSDIR\AspellData.exe" /S /AllUsers'
  ${EndIf}

  # Remove the installer
  Delete "$PLUGINSDIR\AspellData.exe"

  # Aiksarus data
  
  SetOutPath "$INSTDIR\aiksaurus"
  !insertmacro FileListAiksaurusData File "${FILES_AIKSAURUS}\"
  
  # Helper DLLs for NSIS-based tools
  
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListNSISPluginsStandard File "${NSISDIR}\Plugins\"
  !insertmacro FileListNSISPlugins File "${FILES_NSISPLUGINS}\"
  
  # Metafile to EPS Converter
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListMetaFile2EPS File "${FILES_METAFILE2EPS}\"
  
  # Postscript printer for metafile to EPS converter
  SetOutPath "$INSTDIR\PSPrinter"
  !insertmacro FileListPSPrinter File "${FILES_PSPRINTER}\"
  
  # Create uninstaller
  WriteUninstaller "$INSTDIR\${SETUP_UNINSTALLER}"

SectionEnd

#--------------------------------
# Support code for file downloads

!macro DOWNLOAD_FILE RET ID FILENAME APPEND

  # Downloads a file using the InetLoad plug-in (HTTP or FTP)
  
  # RET = Return value (OK if succesful)
  # ID = Name of the download in settings.nsh
  # FILENAME = Location to store file
  # APPEND = Filename to append to server location in settings.nsh

  # Try first mirror server
  InetLoad::load "${DOWNLOAD_${ID}}${APPEND}" "$PLUGINSDIR\${FILENAME}" /END
  Pop ${RET} # Return value (OK if succesful)

  ${If} ${RET} != "OK"
    # Download failed, try second mirror server
    InetLoad::load "${DOWNLOADALT_${ID}}${APPEND}" "$PLUGINSDIR\${FILENAME}" /END
    Pop ${RET}
  ${EndIf}

!macroend

#--------------------------------
# Aspell dictionaries

Var DictionaryFile
Var DictionaryLangName
Var DictionaryLangCode
Var DictionaryPath

Var AspellHive
Var AspellPath

Var DownloadResult

Section -AspellDicts

  # Check whether the system or local version of Aspell should be used
  # The patched Aspell uses the same logic

  ReadRegStr $AspellPath HKCU "Software\Aspell" "Base Path"

  ${If} $AspellPath == ""
    StrCpy $AspellHive HKLM
  ${Else}
    StrCpy $AspellHive HKCU
  ${EndIf}

SectionEnd

!macro SECTION_DICT FILE LANGNAME LANGCODE SIZE

  # One section for each dictionary

  Section /o "${LANGNAME}"

    AddSize ${SIZE}

    StrCpy $DictionaryFile "${FILE}"
    StrCpy $DictionaryLangName "${LANGNAME}"
    StrCpy $DictionaryLangCode ${LANGCODE}

    Call DownloadDictionary

  SectionEnd

!macroend

# Include all sections
!insertmacro Dictionaries '!insertmacro SECTION_DICT'

Function DownloadDictionary

  # Download and install a dictionary

  dict_download:
  
    !insertmacro DOWNLOAD_FILE $DownloadResult ASPELLDICTS aspell6-$DictionaryFile.exe /aspell6-$DictionaryFile.exe

    ${If} $DownloadResult != "OK"
      # Download failed
      MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_DICT) ($DownloadResult)" IDYES dict_download
      Goto dict_noinstall
    ${EndIf}

    install_dict:

      ExecWait '"$PLUGINSDIR\aspell6-$DictionaryFile.exe" /NoDirChange /AutoClose'

      ${If} $AspellHive == HKLM
        ReadRegStr $DictionaryPath HKLM "Software\Aspell\Dictionaries" $DictionaryLangCode
      ${Else}
        ReadRegStr $DictionaryPath HKCU "Software\Aspell\Dictionaries" $DictionaryLangCode
      ${EndIf}

      ${If} $DictionaryPath == ""
        MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_NOTINSTALLED_DICT)" IDYES install_dict
      ${EndIf}

      Delete "$PLUGINSDIR\aspell6-$DictionaryFile.exe"

    dict_noinstall:

FunctionEnd

#--------------------------------
# Extenral components

Var PathAllUsers
Var PathCurrentUser

!macro EXTERNAL COMPONENT

  # Download/Install the component
  
  ${If} $Setup${component} == ${TRUE}
  
    StrCpy $Path${component} "" ;A new one will be installed
  
    !ifndef SETUPTYPE_BUNDLE
      !insertmacro EXTERNAL_DOWNLOAD ${component}
    !else
      !insertmacro EXTERNAL_INSTALL ${component}
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

!ifndef SETUPTYPE_BUNDLE

  !macro EXTERNAL_DOWNLOAD COMPONENT

    download_${COMPONENT}:

      !insertmacro DOWNLOAD_FILE $DownloadResult "${COMPONENT}" "${COMPONENT}Setup.exe" ""
 
      ${If} $DownloadResult != "OK"
        # Download failed
        MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_${COMPONENT}) ($DownloadResult)" IDYES download_${COMPONENT}
        Goto noinstall_${COMPONENT}
      ${EndIf}
      
      !insertmacro EXTERNAL_RUNINSTALLER ${COMPONENT}
      
    noinstall_${COMPONENT}:

  !macroend

!else

  !macro EXTERNAL_INSTALL COMPONENT

    # Extract
    File /oname=$PLUGINSDIR\${COMPONENT}Setup.exe ${FILES_BUNDLE}\${INSTALL_${COMPONENT}}
    
    !insertmacro EXTERNAL_RUNINSTALLER ${COMPONENT}
    
  !macroend

!endif

# Sections for external components

Section -LaTeX ExternalLaTeX
  !insertmacro EXTERNAL LaTeX
SectionEnd

Section -ImageMagick ExternalImageMagick
  !insertmacro EXTERNAL ImageMagick
SectionEnd

Section -Ghostscript ExternalGhostscript
  !insertmacro EXTERNAL Ghostscript
SectionEnd

Function InitExternal

  # Get sizes of external component installers
  
  SectionGetSize ${ExternalLaTeX} $SizeLaTeX
  SectionGetSize ${ExternalImageMagick} $SizeImageMagick
  SectionGetSize ${ExternalGhostscript} $SizeGhostscript
  
  !ifndef SETUPTYPE_BUNDLE
    # Add download size
    IntOp $SizeLaTeX $SizeLaTeX + ${SIZE_DOWNLOAD_LATEX}
    IntOp $SizeImagemagick $SizeImagemagick + ${SIZE_DOWNLOAD_IMAGEMAGICK}
    IntOp $SizeGhostscript $SizeGhostscript + ${SIZE_DOWNLOAD_GHOSTSCRIPT}
  !endif
  
FunctionEnd
