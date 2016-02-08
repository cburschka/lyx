/*
LaTeX.nsh

Handling of LaTeX distributions
*/

# This script contains the following functions:
#
# - LaTeXActions (checks if MiKTeX or TeXLive is installed)
#
# - InstallMiKTeX (installs MiKTeX if not already installed),
#   only for bunlde installer, uses:
#    LaTeXCheck # function from LyXUtils.nsh
#
# - ConfigureMiKTeX
#   (installs the LaTeX class files that are delivered with LyX,
#    a Perl interpreter for splitindex and pdfcrop
#    and enable MiKTeX's automatic package installation)
#
# - ConfigureTeXLive
#   (installs the LaTeX class files that are delivered with LyX)
#
# - UpdateMiKTeX (asks to update MiKTeX)

# ---------------------------------------

Function LaTeXActions
 # checks if MiKTeX or TeXLive is installed

  StrCpy $Is64bit "false"
  
  ${if} ${RunningX64}
   SetRegView 64 # the PATH is in the 64bit registry section
  ${endif}
  # test if MiKTeX is installed
  # reads the PATH variable via the registry because NSIS' "$%Path%" variable is not updated when the PATH changes
  ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
  StrCpy $Search "miktex"
  Call LaTeXCheck # sets the path to the latex.exe to $PathLaTeX # Function from LyXUtils.nsh
  
  StrCpy $String $PathLaTeX
  StrCpy $Search "x64" # search if it is 64bit MiKTeX
  Call StrPoint # Function from LyXUtils.nsh
  ${if} $Pointer != "-1" # if something was found
   StrCpy $Is64bit "true"
  ${endif}
  
  # check for 32bit MiKTeX
  ${if} $PathLaTeX != ""
  ${andif} $Is64bit != "true"
   ${if} ${RunningX64}
    SetRegView 32
   ${endif}
   # check if MiKTeX 2.8 or newer is installed
   StrCpy $0 0
   loop32:
    EnumRegKey $1 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" $0 # check the last subkey
    StrCmp $1 "" done32
    StrCpy $String $1
    IntOp $0 $0 + 1
    Goto loop32
   done32:
   ${if} $String == "2.8"
    StrCpy $MiKTeXVersion "2.8"
    StrCpy $LaTeXName "MiKTeX 2.8"
   ${endif}
   ${if} $String == "2.9"
    StrCpy $MiKTeXVersion "2.9"
    StrCpy $LaTeXName "MiKTeX 2.9"
   ${endif}
  ${endif}
  
  # check for 64bit MiKTeX
  ${if} $LaTeXName == ""
   ${if} ${RunningX64}
    SetRegView 64
   ${endif}
   # check if MiKTeX 2.8 or newer is installed
   StrCpy $0 0
   loop64:
    EnumRegKey $1 HKLM "SOFTWARE\MiKTeX.org\MiKTeX" $0 # check the last subkey
    StrCmp $1 "" done64
    StrCpy $String $1
    IntOp $0 $0 + 1
    Goto loop64
   done64:
   ${if} $String == "2.8"
    StrCpy $MiKTeXVersion "2.8"
    StrCpy $LaTeXName "MiKTeX 2.8"
   ${endif}
   ${if} $String == "2.9"
    StrCpy $MiKTeXVersion "2.9"
    StrCpy $LaTeXName "MiKTeX 2.9"
   ${endif}
  ${endif}
  
  ${if} $PathLaTeX != ""
   StrCpy $MiKTeXUser "HKLM" # needed later to configure MiKTeX
  ${else} # check if MiKTeX is installed only for the current user
   ${if} ${RunningX64}
    SetRegView 64 # the PATH is in the 64bit registry section
   ${endif}
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "miktex"
   Call LaTeXCheck # function from LyXUtils.nsh
   ${if} $PathLaTeX != ""
    StrCpy $MiKTeXUser "HKCU"
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
   ${if} $String == "2.8"
    StrCpy $MiKTeXVersion "2.8"
    StrCpy $LaTeXName "MiKTeX 2.8"
   ${endif}
   ${if} $String == "2.9"
    StrCpy $MiKTeXVersion "2.9"
    StrCpy $LaTeXName "MiKTeX 2.9"
   ${endif}
  ${endif}
  
  ${if} $PathLaTeX != ""
   StrCpy $String $PathLaTeX
   StrCpy $Search "x64" # search if it is 64bit MiKTeX
   Call StrPoint # Function from LyXUtils.nsh
   ${if} $Pointer != "-1" # if something was found
    StrCpy $Is64bit "true"
   ${endif}
   StrCpy $LaTeXInstalled "MiKTeX"
   # on some installations the path ends with a "\" on some not
   # therefore assure that we remove it if it exists
   StrCpy $0 $PathLaTeX "" -1
   ${if} $0 == "\"
    StrCpy $PathLaTeX "$PathLaTeX" -1 # delete "\"
   ${endif}
  ${endif}
  
  # test if TeXLive is installed
  # TeXLive can be installed so that it appears in the PATH variable and/or only as current user.
  # The safest method is to first check for the PATH because this is independent of the TeXLive version.
  ${if} ${RunningX64}
   SetRegView 64 # the PATH is in the 64bit registry section
  ${endif}
  ${if} $PathLaTeX == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
   StrCpy $Search "TeXLive"
   Call LaTeXCheck # function from LyXUtils.nsh
  ${endif}
  # check for the current user Path variable
  ${if} $PathLaTeX == ""
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "texlive"
   StrCpy $2 "TeXLive"
   Call LaTeXCheck # function from LyXUtils.nsh
  ${endif}
  # check if it was installed to the system
  ${if} ${RunningX64}
   SetRegView 32 # TeXLive is a 32bit application
  ${endif}
  ${if} $PathLaTeX == ""
   ReadRegStr $String HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\TeXLive2012" "UninstallString"
   ${if} $String == ""
    ReadRegStr $String HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\TeXLive2012" "UninstallString"
   ${endif}
   ${if} $String != ""
    StrCpy $String $String -28 # remove '\tlpkg\installer\uninst.bat"'
    StrCpy $String $String "" 1 # remove the leading quote
   ${endif}
   StrCpy $PathLaTeX "$String\bin\win32"
   # check if the latex.exe exists in the $PathLaTeX folder
   !insertmacro FileCheck $5 "latex.exe" "$PathLaTeX" # macro from LyXUtils.nsh
   ${if} $5 == "False"
    StrCpy $PathLaTeX ""
   ${endif}
  ${endif}
  # finally set the name
  ${if} $PathLaTeX != ""
  ${andif} $LaTeXName != "MiKTeX 2.8"
  ${andif} $LaTeXName != "MiKTeX 2.9"
   StrCpy $LaTeXInstalled "TeXLive"
   ReadRegStr $String HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\TeXLive2012" "DisplayVersion"
   ${if} $String == ""
    ReadRegStr $String HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\TeXLive2012" "DisplayVersion"
   ${endif}
   ${if} $String != ""
    StrCpy $LaTeXName "TeXLive $String"
   ${else}
    StrCpy $LaTeXName "TeXLive"
   ${endif}
  ${endif}

FunctionEnd

# -------------------------------------------

!if ${SETUPTYPE} == BUNDLE

 Function InstallMiKTeX
  # installs MiKTeX if not already installed
  
  ${if} $PathLaTeX == ""
   # launch MiKTeX's installer
   MessageBox MB_OK|MB_ICONINFORMATION "$(LatexInfo)" /SD IDOK
   MessageBox MB_OK|MB_ICONINFORMATION "${MiKTeXInstall} --shared"
   ${if} $MultiUser.Privileges != "Admin"
   ${andif} $MultiUser.Privileges != "Power"
    # call the non-admin version
    ExecWait ${MiKTeXInstall}
   ${else}
    ExecWait "${MiKTeXInstall} --shared"
   ${endif}
   # test if MiKTeX is installed
   Call LaTeXActions
   ${if} ${RunningX64}
    SetRegView 32 # we install the 32bit version of MiKTeX
   ${endif}
   ${if} $PathLaTeX != ""
    # special entry that it was installed together with LyX
    # so that we can later uninstall it together with LyX
    ${if} $MiKTeXUser == "HKCU"
     WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX" "Yes${APP_SERIES_KEY}"
    ${else}
     WriteRegStr HKLM "SOFTWARE\MiKTeX.org\MiKTeX" "OnlyWithLyX" "Yes${APP_SERIES_KEY}"
    ${endif}
    StrCpy $LaTeXInstalled "MiKTeX"
    StrCpy $MiKTeXVersion ${MiKTeXDeliveredVersion}
   ${else}
    MessageBox MB_OK|MB_ICONSTOP "$(LatexError1)" /SD IDOK
    SetOutPath $TEMP # to be able to delete the $INSTDIR
    RMDir /r $INSTDIR
    Abort
   ${endif} # endif $PathLaTeX != ""
  ${endif}

  FunctionEnd

!endif # endif ${SETUPTYPE} == BUNDLE

# ------------------------------

Function ConfigureMiKTeX
 # installs the LaTeX class files that are delivered with LyX,
 # a Perl interpreter for splitindex and pdfcrop
 # and enable MiKTeX's automatic package installation
 
 # install LyX's LaTeX class and style files and a Perl interpreter
 ${if} $PathLaTeX != ""
  ${if} $MultiUser.Privileges == "Admin"
  ${orif} $MultiUser.Privileges == "Power"
   ${if} $Is64bit == "true"
    StrCpy $PathLaTeXLocal "$PathLaTeX" -15 # delete "\miktex\bin\x64"
   ${else}
    StrCpy $PathLaTeXLocal "$PathLaTeX" -11 # delete "\miktex\bin"
   ${endif}
  ${else}
   StrCpy $PathLaTeXLocal "$APPDATA\MiKTeX\$MiKTeXVersion"
  ${endif}

  # only install the LyX packages if they are not already installed
  ${ifnot} ${FileExists} "$PathLaTeXLocal\tex\latex\lyx\broadway.cls"
   # dvipost
   SetOutPath "$PathLaTeXLocal\tex\latex\dvipost"
   File "${FILES_DVIPOST_PKG}\dvipost.sty"
   # files in Resources\tex
   SetOutPath "$PathLaTeXLocal\tex\latex\lyx"
   CopyFiles /SILENT "$INSTDIR\Resources\tex\*.*" "$PathLaTeXLocal\tex\latex\lyx"
  ${endif}
  # Belarusian support
  ${ifnot} ${FileExists} "$PathLaTeXLocal\tex\generic\babel\belarusian.ldf"
   SetOutPath "$PathLaTeXLocal\tex\generic\babel"
   File "${FILES_DVIPOST_PKG}\belarusian.ldf"
   File "${FILES_DVIPOST_PKG}\belarusian.sty"
  ${endif}
  # Hungarian support
  # this is a replacement therefore do this in every case
  SetOutPath "$PathLaTeXLocal\tex\generic\babel"
  File "${FILES_DVIPOST_PKG}\magyar.ldf"
  
  # install a Perl interpreter for splitindex and pdfcrop
  SetOutPath "$INSTDIR"
  # recursively copy all files under Perl
  File /r "${FILES_PERL}"
  
  # refresh MiKTeX's file name database (do this always to assure everything is in place)
  ${if} $MultiUser.Privileges != "Admin"
  ${andif} $MultiUser.Privileges != "Power"
   # call the non-admin version
   nsExec::ExecToLog "$PathLaTeX\initexmf --update-fndb"
  ${else}
   ${if} $MiKTeXUser != "HKCU" # call the admin version
    nsExec::ExecToLog "$PathLaTeX\initexmf --admin --update-fndb"
   ${else}
    nsExec::ExecToLog "$PathLaTeX\initexmf --update-fndb"
   ${endif}
  ${endif}
  Pop $UpdateFNDBReturn # Return value
  
 ${endif} # end if $PathLaTeX != ""
  
  # enable package installation without asking (1 = Yes, 0 = No, 2 = Ask me first) and
  # if there is not package repository (MiKTeX's primary package repository) then set it
  ${if} ${RunningX64}
  ${andif} $Is64bit == "true"
   SetRegView 64
  ${else}
   SetRegView 32
  ${endif}
  ${if} $MiKTeXUser == "HKCU" # if only for current user
   # if AutoInstall is set to "0" we can assume that this was set purposly since the default is "1"
   ReadRegStr $2 HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall"
   ${if} $2 == "0"
    Goto NoAutoInstall
   ${endif}
   WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall" "1"
   ReadRegStr $1 HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository"
   ${if} $1 == ""
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository" "${MiKTeXRepo}" 
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RepositoryType" "remote"
   ${endif}
  ${else}
   # if AutoInstall is set to "0" we can assume that this was set purposly since the default is "1"
   ReadRegStr $2 HKLM "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall"
   ${if} $2 == "0"
    Goto NoAutoInstall
   ${endif}
   WriteRegStr HKLM "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoInstall" "1"
   ReadRegStr $1 HKLM "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository"
   ${if} $1 == ""
    WriteRegStr HKLM "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RemoteRepository" "${MiKTeXRepo}"
    WriteRegStr HKLM "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "RepositoryType" "remote"
   ${endif}
   # we need to state that missing packages should be installed for all users too
   WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\$MiKTeXVersion\MPM" "AutoAdmin" "t"
  ${endif}
  NoAutoInstall:
  
  # update MiKTeX's package file list
  ExecWait '$PathLaTeX\mpm.exe --update-fndb'
  
  # we must return to 32bit because LyX is a 32bit application
  SetRegView 32
  
FunctionEnd

# ------------------------------

Function ConfigureTeXLive
 # installs the LaTeX class files that are delivered with LyX
 # (TeXLive comes already with a Perl interpreter.)
 
 ${if} $PathLaTeX != ""
  StrCpy $PathLaTeXLocal "$PathLaTeX" -10 # delete "\bin\win32"
  
  # only install the LyX packages if they are not already installed
  ${ifnot} ${FileExists} "$PathLaTeXLocal\texmf-dist\tex\latex\lyx\broadway.cls"
   # dvipost
   SetOutPath "$PathLaTeXLocal\texmf-dist\tex\latex\dvipost"
   File "${FILES_DVIPOST_PKG}\dvipost.sty"
   # files in Resources\tex
   SetOutPath "$PathLaTeXLocal\texmf-dist\tex\latex\lyx"
   CopyFiles /SILENT "$INSTDIR\Resources\tex\*.*" "$PathLaTeXLocal\texmf-dist\tex\latex\lyx"
  ${endif}
 ${endif}
 
 # update TeXLive's package file list
 ExecWait '$PathLaTeX\texhash'
 
 # update TeXLive
 ExecWait '$PathLaTeX\tlmgr update --all'
 
FunctionEnd

# ------------------------------

Function UpdateMiKTeX
 # asks to update MiKTeX

  MessageBox MB_YESNO|MB_ICONINFORMATION "$(MiKTeXInfo)" /SD IDNO IDYES UpdateNow IDNO UpdateLater
  UpdateNow:
  # the update wizard is started by the miktex-update.exe
  ${if} $MultiUser.Privileges != "Admin"
  ${andif} $MultiUser.Privileges != "Power"
   # call the non-admin version
    ExecWait '"$PathLaTeX\internal\miktex-update.exe"'
  ${else}
   ${if} $MiKTeXUser != "HKCU" # call the admin version
    ExecWait '"$PathLaTeX\internal\miktex-update_admin.exe"'
   ${else}
     ExecWait '"$PathLaTeX\internal\miktex-update.exe"'
   ${endif}
  ${endif}
  UpdateLater:

FunctionEnd

