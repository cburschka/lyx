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

  # install GhostScript if not already installed
  Call Ghostscript

  # install ImageMagick if not already installed
  Call ImageMagick

  # install Aspell if not already installed
  Call InstallAspell # function from Aspell.nsh

  # install Aiksaurus if not already installed
  Call Aiksaurus

  # install the LaTeX class files that are delivered with LyX
  # and enable MiKTeX's automatic package installation
  Call ConfigureMiKTeX # Function from LaTeX.nsh

  # install Aspell dictionaries
  ${if} $LangCode == "nb_NO"
   StrCpy $LangCode "no_NO" # we only have a norwegian dictionary available
  ${endif}
  Call InstallAspellDictionary # function from Aspell.nsh

  # configure LyX
  Call ConfigureLyX # Function from ConfigLyX.nsh

  # delete unnecessary files
  ${if} $DelPythonFiles == "True"
   Delete $INSTDIR\bin\python.exe
   Delete $INSTDIR\bin\python25.dll
   Delete $INSTDIR\bin\Python-License.txt
   RMDir /r $INSTDIR\bin\Lib
   RMDir /r $INSTDIR\bin\DLLs
  ${endif}
  RMDir /r $INSTDIR\external

  # create Uninstaller
  WriteUninstaller "${PRODUCT_UNINSTALL_EXE}"

  # run LyX's configure script
  # create a bat-file to start configure in a console window so that the user see the progress
  # of the configuration and to have a signal when the configuration is ready to start LyX
  # this is important when LyX is installed together with MiKTeX or when LyX is installed for the first
  # time on a computer, because the installation of missing LaTeX-files required by LyX could last minutes
  # a batch file is needed because simply calling ExecWait '"$PythonPath\python.exe" "$INSTDIR\Resources\configure.py"'
  # creates the config files in $PythonPath
  ${if} $PythonPath == ""
   StrCpy $PythonPath "$INSTDIR\bin"
  ${endif}
  StrCpy $1 $INSTDIR 2 # get drive letter
  FileOpen $R1 "$INSTDIR\Resources\configLyX.bat" w
  FileWrite $R1 '$1$\r$\n\
  		 cd $INSTDIR\Resources\$\r$\n\
  		 "$PythonPath\python.exe" configure.py'
  FileClose $R1
  MessageBox MB_OK|MB_ICONINFORMATION "$(LatexConfigInfo)"
  ExecWait '"$INSTDIR\Resources\configLyX.bat"'
  Delete "$INSTDIR\Resources\configLyX.bat"

  # ask to update MiKTeX
  ${if} $MiKTeXInstalled == "yes"
   MessageBox MB_YESNO|MB_ICONINFORMATION "$(MiKTeXInfo)" IDYES UpdateNow IDNO UpdateLater
   UpdateNow:
    StrCpy $0 $LaTeXPath -4 # remove "\bin"
    ExecWait '"$LaTeXPath\copystart.exe" "$0\config\update.dat"' # run MiKTeX's update wizard
   UpdateLater:
  ${endif}

  # save MiKTeX's install path to be able to remove LyX's LaTeX-files in the uninstaller
  FileOpen $R1 "$INSTDIR\Resources\uninstallPaths.dat" w
  FileWrite $R1 '$LaTeXPath'
  FileClose $R1

  # prepare variables for uninstaller
  StrCpy $MiKTeXVersionVar ${MiKTeXDeliveredVersion}
  StrCpy $JabRefVersionVar ${JabRefVersion}

SectionEnd

# -------------------------------------------

Function Ghostscript

  # if GhostScript is not installed
  ${if} $GhostscriptPath == ""
   # register Ghostscript
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion}" "GS_DLL" "${GhostscriptDir}\bin\gsdll32.dll"
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion}" "GS_LIB" "${GhostscriptDir}\lib;${GhostscriptDir}\fonts;${GhostscriptDir}\Resource"
   
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
   StrCpy $GhostscriptPath "${GhostscriptDir}\bin"
  ${else}
   # delete unnecessary files
   RMDir /r ${GhostscriptDir}   
  ${endif}

FunctionEnd

# -------------------------------------------

Function ImageMagick

  # if ImageMagick is not installed
  ${if} $ImageMagickPath == ""
   # register ImageMagick
   WriteRegStr HKLM "SOFTWARE\Classes\Applications" "AutoRun" "${ImageMagickDir}\convert.exe $$"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "BinPath" "${ImageMagickDir}"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "CoderModulesPath" "${ImageMagickDir}\modules\coders"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "ConfigurePath" "${ImageMagickDir}\config"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "FilterModulesPath" "${ImageMagickDir}\modules\filters"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\${ImageMagickVersion}\Q:16" "LibPath" "${ImageMagickDir}"
   
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "BinPath" "${ImageMagickDir}"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "CoderModulesPath" "${ImageMagickDir}\modules\coders"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "ConfigurePath" "${ImageMagickDir}\config"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "FilterModulesPath" "${ImageMagickDir}\modules\filters"
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "LibPath" "${ImageMagickDir}"
   WriteRegDWORD HKLM "SOFTWARE\ImageMagick\Current" "QuantumDepth" 0x00000010   
   WriteRegStr HKLM "SOFTWARE\ImageMagick\Current" "Version" "${ImageMagickVersion}"
   
   WriteRegStr HKLM "Software\ImageMagick" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
   StrCpy $ImageMagickPath ${ImageMagickDir}
  ${else}
   # delete unnecessary files
   RMDir /r ${ImageMagickDir}
  ${endif}

FunctionEnd

# -------------------------------------------

Function Aiksaurus

  # if Aiksaurus is not installed
  ${if} $AiksaurusPath == ""
   # extract Aiksaurus' program files
   SetOutPath "$INSTDIR\external"
   File /r "${PRODUCT_SOURCEDIR}\${AiksaurusInstall}"
   # copy the files and register Aiksaurus
   CopyFiles "$INSTDIR\${AiksaurusInstall}" "$APPDATA"
#   WriteRegStr HKLM "Software\Aiksaurus" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
#   WriteRegStr HKLM "Software\Aiksaurus" "Data Path" "${AiksaurusDir}"
  ${endif}

FunctionEnd

