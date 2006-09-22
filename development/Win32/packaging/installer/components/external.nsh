/*

External Components: MiKTeX, ImageMagick, Ghostscript

*/

;--------------------------------
;Macros

!macro SetComponentState var component

  ${if} ${var} == "1"
  
    StrCpy $Setup${component} ${TRUE}
    
    StrCpy $R1 $Size${component}
    
    ${if} $Path${component} == ""
      ;Add size of component itself
      IntOp $R1 $R1 + ${SIZE_${component}}
    ${endif}
    
    SectionSetSize ${External${component}} $R1
    
  ${else}
  
    StrCpy $Setup${component} ${FALSE}
    SectionSetSize ${External${component}} 0
    
  ${endif}

!macroend

!macro ExternalComponent component

  ;Action depending on type of installer
  
  ${if} $Setup${component} == ${TRUE}
  
    StrCpy $Path${component} "" ;A new one will be installed
  
    !ifndef SETUPTYPE_BUNDLE
      !insertmacro DownloadComponent ${component}
    !else
      !insertmacro InstallComponent ${component}
    !endif
    
  ${endif}

!macroend

!macro SetupComponent component

  ;Run the setup application for a component

  install_${component}:
      
    ExecWait '"$PLUGINSDIR\${component}Setup.exe"'
    Call UpdatePathEnvironment
    Call Search${component}
    
    ${if} $Path${component} == ""  
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(TEXT_NOTINSTALLED_${component}) IDYES install_${component}
    ${endif}
      
    Delete "$PLUGINSDIR\${component}Setup.exe"
     
!macroend

!ifndef SETUPTYPE_BUNDLE

  !macro DownloadComponent component

    download_${component}:

      ;Download using HTTP
      NSISdl::download "${DOWNLOAD_${component}}" "$PLUGINSDIR\${component}Setup.exe"
      Pop $R0
 
      ${if} $R0 != "success"
        ;Download failed
        MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_${component}) ($R0)" IDYES download_${component}
        Goto noinstall_${component}
      ${endif}
      
      !insertmacro SetupComponent ${component}
      
    noinstall_${component}:

  !macroend

!else

  !macro InstallComponent component

    ;Extract
    File /oname=$PLUGINSDIR\${component}Setup.exe ${INSTALL_${component}}
    
    !insertmacro SetupComponent ${component}
    
  !macroend

!endif

!macro DialogExternalControl component

  ;Enable/disable the DirRequest control
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${component}.ini" "Field 3" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "external_${component}.ini" "Field 4" "HWND"
  EnableWindow $R1 $R0
  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "external_${component}.ini" "Field 4" "HWND2"
  EnableWindow $R1 $R0

!macroend

!macro DialogExternalShow component

  !insertmacro MUI_HEADER_TEXT $(TEXT_EXTERNAL_${component}_TITLE) $(TEXT_EXTERNAL_${component}_SUBTITLE)
  !insertmacro MUI_INSTALLOPTIONS_INITDIALOG "external_${component}.ini"
  !insertmacro DialogExternalControl ${component}
  !insertmacro MUI_INSTALLOPTIONS_SHOW

!macroend

!macro DialogExternalValidate component

  Push $R0
  Push $R1
  
  ;Next button pressed?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${component}.ini" "Settings" "State"
  ${if} $R0 != "0"
    !insertmacro DialogExternalControl ${component}
    Abort
  ${endif}
  
  ;Download?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${component}.ini" "Field 2" "State"
  !insertmacro SetComponentState $R0 ${component}
  
  ;Folder?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${component}.ini" "Field 3" "State"
  
  ${if} $R0 == "1"
    !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${component}.ini" "Field 4" "State"
    ${unless} ${FileExists} "$R0\${BIN_${component}}"
      MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_EXTERNAL_${component}_NOTFOUND)
      Abort
    ${endif}
    StrCpy $Path${component} $R0
  ${endif}

  Pop $R1
  Pop $R0

!macroend

;--------------------------------
;Sections

Section -LaTeX ExternalLaTeX
  !insertmacro ExternalComponent LaTeX
SectionEnd

Section -ImageMagick ExternalImageMagick
  !insertmacro ExternalComponent ImageMagick
SectionEnd

Section -Ghostscript ExternalGhostscript
  !insertmacro ExternalComponent Ghostscript
SectionEnd

;--------------------------------
;Functions

Function InitSizeExternal

  ;Get sizes of external component installers
  
  SectionGetSize ${ExternalLaTeX} $SizeLaTeX
  SectionGetSize ${ExternalImageMagick} $SizeImageMagick
  SectionGetSize ${ExternalGhostscript} $SizeGhostscript
  
  !ifndef SETUPTYPE_BUNDLE
    ;Add download size
    IntOp $SizeLaTeX $SizeLaTeX + ${SIZE_DOWNLOAD_LATEX}
    IntOp $SizeImagemagick $SizeImagemagick + ${SIZE_DOWNLOAD_IMAGEMAGICK}
    IntOp $SizeGhostscript $SizeGhostscript + ${SIZE_DOWNLOAD_GHOSTSCRIPT}
  !endif
  
FunctionEnd

;--------------------------------
;Page functions

Function PageExternalLaTeX
  !insertmacro DialogExternalShow LaTeX
FunctionEnd

Function PageExternalLaTeXValidate
  !insertmacro DialogExternalValidate LaTeX
FunctionEnd

Function PageExternalImageMagick
  !insertmacro DialogExternalShow ImageMagick
FunctionEnd

Function PageExternalImageMagickValidate
  !insertmacro DialogExternalValidate ImageMagick
FunctionEnd

Function PageExternalGhostscript
  !insertmacro DialogExternalShow Ghostscript
FunctionEnd

Function PageExternalGhostscriptValidate
  !insertmacro DialogExternalValidate Ghostscript
FunctionEnd
