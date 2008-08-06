/*

install.nsh

Installation of program files, dictionaries and external components

*/

#--------------------------------
# Program files

Var PythonCompileFile
Var PythonCompileReturn

Section -ProgramFiles SecProgramFiles

  # Install and register the core LyX files
  
  # The macros are defined in filelists.nsh
  # the parameters are COMMAND DIRECTORY that form command '${COMMAND} "${DIRECTORY}files"  
  
  # Initializes the plug-ins dir ($PLUGINSDIR) if not already initialized.
  # $PLUGINSDIR is automatically deleted when the installer exits.
  InitPluginsDir
  
  # Delete stuff from previous version
  Delete "$INSTDIR\bin\lyxc.exe"
  
  # Binaries
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXBin File "${FILES_LYX}\bin\"
  !insertmacro FileListLyXLauncher File "${FILES_LAUNCHER}\"  
  !insertmacro FileListQtBin File "${FILES_QT}\bin\"
  !insertmacro FileListDll File "${FILES_DEPS}\bin\"
  !insertmacro FileListMSVCBin File "${FILES_MSVC}\"
  !insertmacro FileListMSVCManifest File "..\"
  !insertmacro FileListNetpbmBin File "${FILES_NETPBM}\"
  !insertmacro FileListDTLBin File "${FILES_DTL}\"
  !insertmacro FileListDvipostBin File "${FILES_DVIPOST}\"
  !insertmacro FileListPDFViewBin File "${FILES_PDFVIEW}\"
  !insertmacro FileListPDFToolsBin File "${FILES_PDFTOOLS}\"
  !insertmacro FileListNSISPluginsStandard File "${NSISDIR}\Plugins\"
  !insertmacro FileListNSISPlugins File "${FILES_NSISPLUGINS}\"
  !insertmacro FileListMetaFile2EPS File "${FILES_METAFILE2EPS}\"
  
  # Resources
  SetOutPath "$INSTDIR"
  # recursively copy all files under Resources
  File /r "${FILES_LYX}\Resources"
  
  # Components of Python
  SetOutPath "$INSTDIR\python"
  !insertmacro FileListPythonBin File "${FILES_PYTHON}\"
  !insertmacro FileListMSVCBin File "${FILES_MSVC}\"
  !insertmacro FileListMSVCManifest File "..\"  
  SetOutPath "$INSTDIR\python\Lib"
  !insertmacro FileListPythonLib File "${FILES_PYTHON}\Lib\"
  SetOutPath "$INSTDIR\python\Lib\encodings"
  !insertmacro FileListPythonLibEncodings File "${FILES_PYTHON}\Lib\encodings\"
  
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
  
  !ifdef BUNDLE_IMAGEMAGICK
  # Components of ImageMagick
  SetOutPath "$INSTDIR\imagemagick"
  !insertmacro FileListImageMagick File "${FILES_IMAGEMAGICK}\"
  !insertmacro FileListMSVCBin File "${FILES_MSVC}\"
  !insertmacro FileListMSVCManifest File "..\"
  !endif  
  
  !ifdef BUNDLE_GHOSTSCRIPT
  # Components of Ghostscript
  SetOutPath "$INSTDIR\ghostscript"
  SetOutPath "$INSTDIR\ghostscript\bin"
  !insertmacro FileListGhostscriptBin File "${FILES_GHOSTSCRIPT}\bin\"
  !insertmacro FileListMSVCBin File "${FILES_MSVC}\"
  !insertmacro FileListMSVCManifest File "..\"  
  SetOutPath "$INSTDIR\ghostscript\lib"
  !insertmacro FileListGhostscriptLib File "${FILES_GHOSTSCRIPT}\lib\"
  SetOutPath "$INSTDIR\ghostscript\fonts"
  !insertmacro FileListGhostscriptFonts File "${FILES_GHOSTSCRIPT}\fonts\"
  SetOutPath "$INSTDIR\ghostscript\Resource"
  SetOutPath "$INSTDIR\ghostscript\Resource\CMap"
  !insertmacro FileListGhostscriptResourceCMap File "${FILES_GHOSTSCRIPT}\Resource\CMap\"
  SetOutPath "$INSTDIR\ghostscript\Resource\ColorSpace"
  !insertmacro FileListGhostscriptResourceColorSpace File "${FILES_GHOSTSCRIPT}\Resource\ColorSpace\"
  SetOutPath "$INSTDIR\ghostscript\Resource\Decoding"
  !insertmacro FileListGhostscriptResourceDecoding File "${FILES_GHOSTSCRIPT}\Resource\Decoding\"
  SetOutPath "$INSTDIR\ghostscript\Resource\Encoding"
  !insertmacro FileListGhostscriptResourceEncoding File "${FILES_GHOSTSCRIPT}\Resource\Encoding\"
  
  !endif  
  
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
  
  # Postscript printer for metafile to EPS converter
  SetOutPath "$INSTDIR\PSPrinter"
  !insertmacro FileListPSPrinter File "${FILES_PSPRINTER}\"
  
  # Create uninstaller
  WriteUninstaller "$INSTDIR\${SETUP_UNINSTALLER}"

SectionEnd

#--------------------------------
# Support code for file downloads

!macro DOWNLOAD_FILE RET ID FILENAME APPEND

  # Downloads a file using the Inetc plug-in (HTTP or FTP)
  
  # RET = Return value (OK if succesful)
  # ID = Name of the download in settings.nsh
  # FILENAME = Location to store file
  # APPEND = Filename to append to server location in settings.nsh

  # Try first mirror server
  Inetc::get "${DOWNLOAD_${ID}}${APPEND}" "$PLUGINSDIR\${FILENAME}" /END
  Pop ${RET} # Return value (OK if succesful)

  ${If} ${RET} != "OK"
    # Download failed, try second mirror server
    Inetc::get "${DOWNLOADALT_${ID}}${APPEND}" "$PLUGINSDIR\${FILENAME}" /END
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
 
    ${If} $DownloadResult != "OK"
      # Download failed
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

!ifndef BUNDLE_IMAGEMAGICK

Section -ImageMagick ExternalImageMagick
  !insertmacro EXTERNAL ImageMagick
SectionEnd

!endif

!ifndef BUNDLE_GHOSTSCRIPT

Section -Ghostscript ExternalGhostscript
  !insertmacro EXTERNAL Ghostscript
SectionEnd

!endif

Function InitExternal

  # Get sizes of external component installers
  
  SectionGetSize ${ExternalLaTeX} $SizeLaTeX
  !ifndef BUNDLE_IMAGEMAGICK
    SectionGetSize ${ExternalImageMagick} $SizeImageMagick
  !endif
  !ifndef BUNDLE_GHOSTSCRIPT
    SectionGetSize ${ExternalGhostscript} $SizeGhostscript
  !endif
  
  # Add download size
  
  !ifndef BUNDLESETUP_MIKTEX
    IntOp $SizeLaTeX $SizeLaTeX + ${SIZE_DOWNLOAD_LATEX}
  !endif
  !ifndef BUNDLE_IMAGEMAGICK & BUNDLESETUP_IMAGEMAGICK
    IntOp $SizeImagemagick $SizeImagemagick + ${SIZE_DOWNLOAD_IMAGEMAGICK}
  !endif
  !ifndef BUNDLE_GHOSTSCRIPT & BUNDLESETUP_GHOSTSCRIPT
    IntOp $SizeGhostscript $SizeGhostscript + ${SIZE_DOWNLOAD_GHOSTSCRIPT}
  !endif

FunctionEnd
