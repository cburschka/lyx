Function LaTeXActions
 # check if MiKTeX or TeXLive is installed

  # test if MiKTeX is installed
  # reads the PATH variable via the registry because NSIS' "$%Path%" variable is not updated when the PATH changes
  ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
  StrCpy $Search "miktex"
  Call LaTeXCheck # sets the path to the latex.exe to $LatexPath # Function from LyXUtils.nsh
  
  ${if} $LatexPath != ""
   # check if MiKTeX 2.5 or 2.6 and newer is installed
   StrCpy $0 0
   loopA:
    EnumRegKey $1 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" $0 # check the last subkey
    StrCmp $1 "" doneA
    StrCpy $String $1
    IntOp $0 $0 + 1
    Goto loopA
   doneA:
   ${if} $String == "2.5"
    StrCpy $MiKTeXVersion "2.5"
    StrCpy $LaTeXName "MiKTeX 2.5"
   ${endif}
   ${if} $String == "2.6"
    StrCpy $MiKTeXVersion "2.6"
    StrCpy $LaTeXName "MiKTeX 2.6"
   ${endif}
   ${if} $String == "2.7"
    StrCpy $MiKTeXVersion "2.7"
    StrCpy $LaTeXName "MiKTeX 2.7"
   ${endif}
  ${endif}
  
  ${if} $LatexPath == "" # check if MiKTeX is installed only for the current user
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "miktex"
   Call LaTeXCheck # function from LyXUtils.nsh
   ${if} $LatexPath != ""
    StrCpy $MiKTeXUser "HKCU" # needed later to configure MiKTeX
   ${endif}
  ${endif}
  ${if} $LaTeXName == "" # check for the MiKTeX version
   StrCpy $0 0
   loopB:
    EnumRegKey $1 HKCU "SOFTWARE\MiKTeX.org\MiKTeX" $0 # check the last subkey
    StrCmp $1 "" doneB
    StrCpy $String $1
    IntOp $0 $0 + 1
    Goto loopB
   doneB:
   ${if} $String == "2.5"
    StrCpy $MiKTeXVersion "2.5"
    StrCpy $LaTeXName "MiKTeX 2.5"
   ${endif}
   ${if} $String == "2.6"
    StrCpy $MiKTeXVersion "2.6"
    StrCpy $LaTeXName "MiKTeX 2.6"
   ${endif}
   ${if} $String == "2.7"
    StrCpy $MiKTeXVersion "2.7"
    StrCpy $LaTeXName "MiKTeX 2.7"
   ${endif}
  ${endif}
    
  ${if} $LatexPath != ""
   StrCpy $MiKTeXInstalled "yes"
  ${endif}
  
  # test if TeXLive is installed
  # as described at TeXLives' homepage there should be an entry in the PATH
  ${if} $LatexPath == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
   StrCpy $Search "TeXLive"
   Call LaTeXCheck # function from LyXUtils.nsh
  ${endif}
  # check for the current user Path variable (the case when it is a live CD/DVD)
  ${if} $LatexPath == ""
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "texlive"
   StrCpy $2 "TeXLive"
   Call LaTeXCheck # function from LyXUtils.nsh
  ${endif}
  # check if the variable TLroot exists (the case when it is installed using the program "tlpmgui")
  ${if} $LatexPath == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TLroot"
   ${if} $String == ""
    ReadRegStr $String HKCU "Environment" "TLroot" # the case when installed without admin permissions
   ${endif}
   StrCpy $LatexPath "$String\bin\win32"
   # check if the latex.exe exists in the $LatexPath folder
   !insertmacro FileCheck $5 "latex.exe" "$LatexPath" # macro from LyXUtils.nsh
   ${if} $5 == "False"
    StrCpy $LatexPath ""
   ${endif}
  ${endif}
  ${if} $LatexPath != ""
  ${andif} $LaTeXName != "MiKTeX 2.5"
  ${andif} $LaTeXName != "MiKTeX 2.6"
  ${andif} $LaTeXName != "MiKTeX 2.7"
   StrCpy $LaTeXName "TeXLive"
  ${endif}
  
  ${if} $LatexPath == ""
   StrCpy $MissedProg "True"
  ${endif}

FunctionEnd

# -------------------------------------------

!if ${INSTALLER_VERSION} == "Complete"

 Function InstallMiKTeX
  # install MiKTeX if not already installed

  ${if} $LatexPath == ""
   # launch MiKTeX's installer
   MessageBox MB_OK|MB_ICONINFORMATION "$(LatexInfo)"
   ExecWait ${MiKTeXInstall}
   # test if MiKTeX is installed
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
   StrCpy $Search "miktex"
   Call LaTeXCheck
   ${if} $LatexPath == ""
    StrCpy $MiKTeXUser "HKCU"
    ReadRegStr $String HKCU "Environment" "Path"
    StrCpy $Search "miktex"
    Call LaTeXCheck
   ${endif}
   ${if} $LatexPath != ""
    # set package repository (MiKTeX's primary package repository)
    WriteRegStr HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
    StrCpy $MiKTeXInstalled "yes"
    StrCpy $MiKTeXVersion ${MiKTeXDeliveredVersion}
    ${if} $MiKTeXUser != "HKCU"
     StrCpy $MiKTeXPath "$LatexPath" -11 # delete "\miktex\bin"
     #MessageBox MB_OK|MB_ICONINFORMATION "$(MiKTeXPathInfo)" # info that MiKTeX's installation folder must have write permissions for all users to work properly
    ${endif}
   ${else}
    MessageBox MB_OK|MB_ICONSTOP "$(LatexError1)"
    SetOutPath $TEMP # to be able to delete the $INSTDIR
    RMDir /r $INSTDIR
    Abort
   ${endif} # endif $LatexPath != ""
  ${endif}

  FunctionEnd

!endif # endif ${INSTALLER_VERSION} == "Complete"

# ------------------------------

Function ConfigureMiKTeX
 # installs the LaTeX class files that are delivered with LyX
 # and enable MiKTeX's automatic package installation
 
  StrCpy $String $LatexPath
  StrCpy $Search "miktex\bin"
  StrLen $3 $String
  Call StrPoint # search the LaTeXPath for the phrase "miktex\bin" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" # if something was found
   IntOp $Pointer $Pointer - 1 # jump before the first "\" of "\miktex\bin"
   StrCpy $String $String "$Pointer" # $String is now the part before "\miktex\bin"
   # install LyX's special LaTeX class files
   CreateDirectory "$String\tex\latex\lyx"
   SetOutPath "$String\tex\latex\lyx"
   File "${ClassFileDir}\lyxchess.sty"
   File "${ClassFileDir}\lyxskak.sty"
   CreateDirectory "$String\tex\latex\revtex"
   SetOutPath "$String\tex\latex\revtex"
   File "${ClassFileDir}\revtex.cls"
   CreateDirectory "$String\tex\latex\hollywood"
   SetOutPath "$String\tex\latex\hollywood"
   File "${ClassFileDir}\hollywood.cls"
   CreateDirectory "$String\tex\latex\broadway"
   SetOutPath "$String\tex\latex\broadway"
   File "${ClassFileDir}\broadway.cls"
   # install LaTeX-package dvipost (dvipost is not available for MiKTeX)
   SetOutPath "$String\tex\latex"
   File /r "${DVIPostFileDir}"

   # refresh MiKTeX's file name database
   ExecWait "$LaTeXPath\initexmf --update-fndb"
    
   ${if} $MiKTeXVersion == "2.5"
    # enable package installation without asking (t = Yes, f = No)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall" "1" # if only for current user
    ${if} $MiKTeXUser != "HKCU"
     WriteRegStr SHCTX "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_AUTOINSTALL" "t"
    ${endif}
    # set package repository (MiKTeX's primary package repository)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository" "${MiKTeXRepo}" # if only for current user
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RepositoryType" "remote" # if only for current user
    ${if} $MiKTeXUser != "HKCU"
     WriteRegStr SHCTX "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_REPOSITORY" "${MiKTeXRepo}"
    ${endif}
   
   ${else} # if MiKTeX 2.6 or above
    # enable package installation without asking (1 = Yes, 0 = No, 2 = Ask me first)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall" "1" # if only for current user
    ${if} $MiKTeXUser != "HKCU"
     WriteRegStr SHCTX "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall" "1"
    ${endif}
    # set package repository (MiKTeX's primary package repository)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository" "${MiKTeXRepo}" # if only for current user
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RepositoryType" "remote" # if only for current user
    ${if} $MiKTeXUser != "HKCU"
     WriteRegStr SHCTX "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository" "${MiKTeXRepo}"
     WriteRegStr SHCTX "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RepositoryType" "remote"
    ${endif}
    
   ${endif} # end if $MiKTeXVersion == "2.5"
   
   # enable MiKTeX's automatic package installation
   ${if} $MiKTeXVersion == "2.5"
    ExecWait '$LaTeXPath\mpm.com --update-fndb'
   ${else} # MiKTeX 2.6 or newer
    ExecWait '$LaTeXPath\mpm.exe --update-fndb'
   ${endif}
   # the following feature is planned to be used for a possible CD-version
   # copy LaTeX-packages needed by LyX
   # SetOutPath "$INSTDIR"
   # File /r "${LaTeXPackagesDir}" 
  ${endif} # end ${if} $Pointer
  
  # save MiKTeX's install path to be able to remove LyX's LaTeX-files in the uninstaller
  FileOpen $R1 "$INSTDIR\Resources\uninstallPaths.dat" w
  FileWrite $R1 '$LaTeXPath'
  FileClose $R1
  
FunctionEnd

Function UpdateMiKTeX
 # ask to update MiKTeX

  ${if} $MiKTeXInstalled == "yes"
   MessageBox MB_YESNO|MB_ICONINFORMATION "$(MiKTeXInfo)" IDYES UpdateNow IDNO UpdateLater
   UpdateNow:
    StrCpy $0 $LaTeXPath -4 # remove "\bin"
    # the update wizard is either started by the copystart.exe or the copystart_admin.exe
    # the latter replaces copystart.exe since miktex-2.6.2742
    ExecWait '"$LaTeXPath\copystart.exe" "$0\config\update.dat"' # run MiKTeX's update wizard
    ExecWait '"$LaTeXPath\copystart_admin.exe" "$0\config\update.dat"' # run MiKTeX's update wizard
   UpdateLater:
  ${endif}

FunctionEnd

