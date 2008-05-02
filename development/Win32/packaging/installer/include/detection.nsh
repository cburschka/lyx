/*

detection.nsh

Detection of external component locations

*/

Function SearchExternal
  Call SearchLaTeX
  Call SearchBibTeXEditor
  !ifndef BUNDLE_IMAGEMAGICK
    Call SearchImageMagick
  !endif
  !ifndef BUNDLE_GHOSTSCRIPT
    Call SearchGhostscript
  !endif
FunctionEnd

#--------------------------------
# MiKTeX

Var ReportReturn
Var CommandLineOutput
Var LastChar
Var PathLength

!macro SEARCH_MIKTEX25

  # Search location of MiKTeX installation using initexmf
  # Works for version 2.5 and later
  
  nsExec::ExecToStack "initexmf.exe --report"
  Pop $ReportReturn
  Pop $CommandLineOutput

  ${WordFind2X} $CommandLineOutput "BinDir: " "$\r" "+1" $PathLaTeX
  ${WordFind2X} $CommandLineOutput "CommonData: " "$\r" "+1" $PathLaTeXLocal # Local root  

!macroend

!macro SEARCH_MIKTEX24 ROOTKEY

  ReadRegStr $PathLaTeX ${ROOTKEY} "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Install Root"
  
  ${If} $PathLaTeX != ""
    StrCpy $LastChar $PathLaTeX 1 -1
    
    ${If} $LastChar == "\"
      # Trim backslash
      StrLen $PathLength $PathLaTeX
      IntOp $PathLength $PathLength - 1
      StrCpy $PathLaTeX $PathLaTeX $PathLength
    ${EndIf}
    
    StrCpy $PathLaTeX "$PathLaTeX\miktex\bin"
     
    #Local root
    ReadRegStr $PathLaTeXLocal ${ROOTKEY} "Software\MiK\MiKTeX\CurrentVersion\MiKTeX" "Local Root"
    
  ${EndIf}
   
!macroend

Function SearchLaTeX

  # Search where MikTeX is installed
  
  !insertmacro SEARCH_MIKTEX25
  
  ${IfNot} ${FileExists} "$PathLaTeX\${BIN_LATEX}"
    !insertmacro SEARCH_MIKTEX24 HKCU
  ${EndIf}

  ${IfNot} ${FileExists} "$PathLaTeX\${BIN_LATEX}"
    !insertmacro SEARCH_MIKTEX24 HKLM
  ${EndIf}

  ${IfNot} ${FileExists} "$PathLaTeX\${BIN_LATEX}"
    StrCpy $PathLateX ""
  ${EndIf}

FunctionEnd

#--------------------------------
# Ghostscript

!ifndef BUNDLE_GHOSTSCRIPT

!insertmacro GetParent
!insertmacro VersionCompare

Var Counter
Var EnumReturn
Var CompareReturn
Var AFPLVersion
Var GPLVersion

Function SearchGhostscript

  # Search where Ghostscript is installed
  
  # Find the latest version of AFPL Ghostscript installed
  
  StrCpy $Counter 0
    
  ${do}
  
    EnumRegKey $EnumReturn HKLM "Software\AFPL Ghostscript" $Counter
    
    ${If} $EnumReturn != ""
      ${VersionCompare} $EnumReturn $AFPLVersion $CompareReturn
      ${If} $CompareReturn == "1"
        StrCpy $AFPLVersion $EnumReturn
      ${EndIf}
      IntOp $Counter $Counter + 1
    ${EndIf}
    
  ${loopuntil} $EnumReturn == ""
    
  # The same for GPL Ghostscript
    
  StrCpy $Counter 0
  
  ${do}

    EnumRegKey $EnumReturn HKLM "Software\GPL Ghostscript" $Counter
    
    ${If} $EnumReturn != ""
      ${VersionCompare} $EnumReturn $GPLVersion $CompareReturn
      ${If} $CompareReturn == "1"
        StrCpy $GPLVersion $EnumReturn
      ${EndIf}
      IntOp $Counter $Counter + 1
    ${EndIf}
  
  ${loopuntil} $EnumReturn == ""  
  
  # Take the latest one
  ${VersionCompare} $AFPLVersion $GPLVersion $CompareReturn

  ${If} $CompareReturn = 1
    # AFPL is newer
    ReadRegStr $PathGhostscript HKLM "Software\AFPL Ghostscript\$R3" "GS_DLL"
  ${Else}
    # GPL is newer or equal
    ReadRegStr $PathGhostscript HKLM "Software\GPL Ghostscript\$R4" "GS_DLL"
  ${EndIf}
  
  # Trim the DLL filename to get the path
  ${GetParent} $PathGhostscript $PathGhostscript
  
  ${IfNot} ${FileExists} "$PathGhostscript\${BIN_GHOSTSCRIPT}"
    StrCpy $PathGhostscript ""  
  ${EndIf}
  
FunctionEnd

!endif

#--------------------------------
# ImageMagick

!ifndef BUNDLE_IMAGEMAGICK

Function SearchImageMagick

  # Search where ImageMagick is installed
  ReadRegStr $PathImageMagick HKLM "Software\ImageMagick\Current" "BinPath"
  
  ${IfNot} ${FileExists} "$PathImageMagick\${BIN_IMAGEMAGICK}"
    StrCpy $PathImageMagick ""  
  ${EndIf}

FunctionEnd

!endif

#--------------------------------
# JabRef

Function SearchBibTeXEditor

  # Search where JabRef is installed
  ReadRegStr $PathBibTeXEditor HKCU "Software\JabRef" "Path"

  ${IfNot} ${FileExists} "$PathBibTeXEditor\${BIN_BIBTEXEDITOR}"
    ReadRegStr $PathBibTeXEditor HKLM "Software\JabRef" "Path"
  ${EndIf}

  ${IfNot} ${FileExists} "$PathBibTeXEditor\${BIN_BIBTEXEDITOR}"
    StrCpy $PathBibTeXEditor ""  
  ${EndIf}

FunctionEnd
