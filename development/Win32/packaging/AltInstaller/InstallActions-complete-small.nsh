# this file contains the main installer section

 # The '-' makes the section invisible.
 Section "-Installation actions" SecInstallation

  # dummy actions to avoid NSIS warnings
  StrCpy $FileName ""
  StrCpy $NewString ""
  StrCpy $OldString ""
  
  # extract all files
  SetOutPath "$INSTDIR"
  File /r "${PRODUCT_SOURCEDIR}\bin"
  File /r "${PRODUCT_SOURCEDIR}\etc"
  File /r "${PRODUCT_SOURCEDIR}\Resources"
  !if ${INSTALLER_VERSION} == "Complete"
   File /r "${PRODUCT_SOURCEDIR}\external"
  !endif

  !if ${INSTALLER_VERSION} == "Complete"
   # install MiKTeX if not already installed
   Call InstallMiKTeX # function from LaTeX.nsh
  !endif

  # install Ghostscript if not already installed
  Call Ghostscript # function from InstallThirdPartyProgs.nsh

  # install ImageMagick if not already installed
  Call ImageMagick # function from InstallThirdPartyProgs.nsh

  # install Aspell if not already installed
  Call InstallAspell # function from Aspell.nsh

  # install Aiksaurus if not already installed
  Call Aiksaurus # function from InstallThirdPartyProgs.nsh
  
  # install Metafile2eps if not already installed
  Call Metafile2eps # function from InstallThirdPartyProgs.nsh

  !if ${INSTALLER_VERSION} == "Complete"
   # install GSview if not already installed and the user selected it
   Call PostScript # function from InstallThirdPartyProgs.nsh
   
   # install JabRef if not already installed and the user selected it
   Call BibTeX # function from InstallThirdPartyProgs.nsh
  !endif

  # install the LaTeX class files that are delivered with LyX
  # and enable MiKTeX's automatic package installation
  Call ConfigureMiKTeX # Function from LaTeX.nsh

  # refresh shell, update the environment variables otherwise the first
  # configure run will fail when MiKTeX was just installed
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'

  # install Aspell dictionaries
  Call InstallAspellDictionary # function from Aspell.nsh

  # configure LyX
  Call ConfigureLyX # Function from ConfigLyX.nsh

  # create Uninstaller
  WriteUninstaller "${PRODUCT_UNINSTALL_EXE}"

  # run LyX's configure script
  MessageBox MB_OK|MB_ICONINFORMATION "$(LatexConfigInfo)"
  ExecWait '"$INSTDIR\Resources\configLyX.bat"'
  Delete "$INSTDIR\Resources\configLyX.bat"

  # ask to update MiKTeX
  Call UpdateMiKTeX # function from latex.nsh

 SectionEnd

