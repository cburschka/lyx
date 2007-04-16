Function LaTeXActions
 ; tests if MiKTeX is installed
 ; reads the PATH variable via the registry because NSIS' "$%Path%" variable is not updated when the PATH changes
 
  ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
  StrCpy $Search "miktex"
  Call LaTeXCheck ; sets the path to the latex.exe to $LatexPath ; Function from LyXUtils.nsh
  ; check if MiKTeX 2.4 or 2.5 is installed
  StrCpy $String ""
  ReadRegStr $String HKLM "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Install Root"
  ${if} $String != ""
   StrCpy $MiKTeXVersion "2.4" ; needed later for the configuration of MiKTeX
   StrCpy $LaTeXName "MiKTeX 2.4"
  ${endif}
  
  ${if} $LatexPath == "" ; check if MiKTeX is installed only for the current user
   ; check for MiKTeX 2.5
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "miktex"
   Call LaTeXCheck ; function from LyXUtils.nsh
   ${if} $LatexPath != ""
    StrCpy $MiKTeXUser "HKCU" ; needed later to for a message about MiKTeX's install folder write permissions, see InstallActions-*.nsh
   ${endif}
   ; check for MiKTeX 2.4
   StrCpy $String ""
   ReadRegStr $String HKCU "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Install Root"
   ${if} $String != ""
    StrCpy $MiKTeXVersion "2.4"
    StrCpy $LaTeXName "MiKTeX 2.4"
   ${endif}
  ${endif}
  
  ${if} $LatexPath != ""
   StrCpy $MiKTeXInstalled "yes"
   ${if} $LaTeXName != "MiKTeX 2.4"
    StrCpy $LaTeXName "MiKTeX 2.5"
   ${endif} 
  ${endif}

  ; test if TeXLive is installed
  ; as described at TeXLives' homepage there should be an entry in the PATH
  ${if} $LatexPath == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "Path"
   StrCpy $Search "TeXLive"
   Call LaTeXCheck ; function from LyXUtils.nsh
  ${endif}
  ; check for the current user Path variable (the case when it is a live CD/DVD)
  ${if} $LatexPath == ""
   ReadRegStr $String HKCU "Environment" "Path"
   StrCpy $Search "texlive"
   StrCpy $2 "TeXLive"
   Call LaTeXCheck ; function from LyXUtils.nsh
  ${endif}
  ; check if the variable TLroot exists (the case when it is installed using the program "tlpmgui")
  ${if} $LatexPath == ""
   ReadRegStr $String HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TLroot"
   ${if} $String == ""
    ReadRegStr $String HKCU "Environment" "TLroot" ; the case when installed without admin permissions
   ${endif}
   StrCpy $LatexPath "$String\bin\win32"
   ; check if the latex.exe exists in the $LatexPath folder
   !insertmacro FileCheck $5 "latex.exe" "$LatexPath" ; macro from LyXUtils.nsh
   ${if} $5 == "False"
    StrCpy $LatexPath ""
   ${endif}
  ${endif}
  ${if} $LatexPath != ""
  ${andif} $LaTeXName != "MiKTeX 2.4"
  ${andif} $LaTeXName != "MiKTeX 2.5"
   StrCpy $LaTeXName "TeXLive"
  ${endif}
  
FunctionEnd

; ------------------------------

Function ConfigureMiKTeX
 ; installs the LaTeX class files that are delivered with LyX
 ; and enable MiKTeX's automatic package installation
 
  StrCpy $String $LatexPath
  StrCpy $Search "miktex\bin"
  StrLen $3 $String
  Call StrPoint ; search the LaTeXPath for the phrase "miktex\bin" (function from LyXUtils.nsh)
  ${if} $Pointer != "-1" ; if something was found
   IntOp $Pointer $Pointer - 1 ; jump before the first "\" of "\miktex\bin"
   StrCpy $String $String "$Pointer" ; $String is now the part before "\miktex\bin"
   ; install LaTeX class files
   SetOutPath "$String\tex\latex"
   File "${ClassFileDir}\cv.cls"
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
   ; install LaTeX-package dvipost (dvipost is not available for MiKTeX)
   SetOutPath "$String\tex\latex\"
   File /r "${DVIPostFileDir}"

   ${if} $MiKTeXVersion == "2.4"
    ; refresh MiKTeX's file name database
    ExecWait "$String\miktex\bin\initexmf --update-fndb"
    ; delete MiKTeX 2.4's dvipng executable as it is an old broken version. Then install a working one.
    Delete "$String\miktex\bin\dvipng.exe"
    ; Install a new one
    SetOutPath "$String\miktex\bin"
    File "${PRODUCT_DIR}\LyX\external\dvipng.exe"
    ; enable package installation without asking (1=Yes, 0=No, 2=Always Ask Before Installing)						    
    WriteRegStr HKCU "SOFTWARE\MiK\MiKTeX\CurrentVersion\MiKTeX" "InstallPackagesOnTheFly" "1"
    WriteRegStr HKCU "SOFTWARE\MiK\MiKTeX\CurrentVersion\MPM\Settings" "" ""
    ; Setting package repository (MiKTeX's primary package repository)
    WriteRegStr HKCU "SOFTWARE\MiK\MiKTeX\CurrentVersion\MPM" "RemotePackageRepository" "${MiKTeXRepo}"
   
   ${else} ; if MiKTeX 2.5
    ; refresh MiKTeX's file name database
    ExecWait "$LaTeXPath\initexmf --update-fndb"
    ; enable package installation without asking (t = Yes, f = No)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\2.5\MPM" "AutoInstall" "1" ; if only for curent user
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_AUTOINSTALL" "t"
    ; set package repository (MiKTeX's primary package repository)
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\2.5\MPM" "RemoteRepository" "${MiKTeXRepo}" ; if only for curent user
    WriteRegStr HKCU "SOFTWARE\MiKTeX.org\MiKTeX\2.5\MPM" "RepositoryType" "remote" ; if only for curent user
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "MIKTEX_REPOSITORY" "${MiKTeXRepo}"
   ${endif}
   
   ; enable MiKTeX's automatic package installation
   ExecWait '$LaTeXPath\mpm.com --update-fndb'
   ; the following feature is planned to be used for a possible CD-version
   ; copy LaTeX-packages needed by LyX
   ; SetOutPath "$INSTDIR"
   ; File /r "${LaTeXPackagesDir}" 
  ${endif} ; end ${if} $Pointer
  
FunctionEnd
