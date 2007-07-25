/*

External Components: MiKTeX, ImageMagick, Ghostscript

*/

#--------------------------------
#Macros

!macro SetComponentState VAR COMPONENT

  ${if} ${VAR} == "1"
  
    StrCpy $Setup${COMPONENT} ${TRUE}
    
    StrCpy $R1 $Size${COMPONENT}
    
    ${if} $Path${COMPONENT} == ""
      #Add size of component itself
      IntOp $R1 $R1 + ${SIZE_${COMPONENT}}
    ${endif}
    
    SectionSetSize ${External${COMPONENT}} $R1
    
  ${else}
  
    StrCpy $Setup${COMPONENT} ${FALSE}
    SectionSetSize ${External${COMPONENT}} 0
    
  ${endif}

!macroend

!macro ExternalComponent COMPONENT

  #Action depending on type of installer
  
  ${if} $Setup${component} == ${TRUE}
  
    StrCpy $Path${component} "" ;A new one will be installed
  
    !ifndef SETUPTYPE_BUNDLE
      !insertmacro DownloadComponent ${component}
    !else
      !insertmacro InstallComponent ${component}
    !endif
    
  ${endif}

!macroend

!macro SetupComponent COMPONENT

  #Run the setup application for a component

  install_${COMPONENT}:
      
    ExecWait '"$PLUGINSDIR\${COMPONENT}Setup.exe"'
    Call UpdatePathEnvironment
    Call Search${COMPONENT}
    
    ${if} $Path${COMPONENT} == ""  
      MessageBox MB_YESNO|MB_ICONEXCLAMATION $(TEXT_NOTINSTALLED_${COMPONENT}) IDYES install_${COMPONENT}
    ${endif}
      
    Delete "$PLUGINSDIR\${COMPONENT}Setup.exe"
     
!macroend

!ifndef SETUPTYPE_BUNDLE

  !macro DownloadComponent COMPONENT

    download_${COMPONENT}:

      !insertmacro DownloadFile $R0 "${COMPONENT}" "${COMPONENT}Setup.exe" ""
 
      ${if} $R0 != "OK"
        #Download failed
        MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(TEXT_DOWNLOAD_FAILED_${COMPONENT}) ($R0)" IDYES download_${COMPONENT}
        Goto noinstall_${COMPONENT}
      ${endif}
      
      !insertmacro SetupComponent ${COMPONENT}
      
    noinstall_${COMPONENT}:

  !macroend

!else

  !macro InstallComponent COMPONENT

    #Extract
    File /oname=$PLUGINSDIR\${COMPONENT}Setup.exe ${FILES_BUNDLE}\${INSTALL_${COMPONENT}}
    
    !insertmacro SetupComponent ${COMPONENT}
    
  !macroend

!endif

!macro DialogExternalControl component

  #Enable/disable the DirRequest control
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${component}.ini" "Field 3" "State"
  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "external_${component}.ini" "Field 4" "HWND"
  EnableWindow $R1 $R0
  !insertmacro MUI_INSTALLOPTIONS_READ $R1 "external_${component}.ini" "Field 4" "HWND2"
  EnableWindow $R1 $R0

!macroend

!macro DialogExternalShow COMPONENT

  !insertmacro MUI_HEADER_TEXT $(TEXT_EXTERNAL_${COMPONENT}_TITLE) $(TEXT_EXTERNAL_${COMPONENT}_SUBTITLE)
  !insertmacro MUI_INSTALLOPTIONS_INITDIALOG "external_${COMPONENT}.ini"
  !insertmacro DialogExternalControl ${COMPONENT}
  !insertmacro MUI_INSTALLOPTIONS_SHOW

!macroend

!macro DialogExternalValidate COMPONENT

  Push $R0
  Push $R1
  
  #Next button pressed?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${COMPONENT}.ini" "Settings" "State"
  ${if} $R0 != "0"
    !insertmacro DialogExternalControl ${COMPONENT}
    Abort
  ${endif}
  
  #Download?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${COMPONENT}.ini" "Field 2" "State"
  !insertmacro SetComponentState $R0 ${COMPONENT}
  
  #Folder?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${COMPONENT}.ini" "Field 3" "State"
  
  ${if} $R0 == "1"
    !insertmacro MUI_INSTALLOPTIONS_READ $R0 "external_${COMPONENT}.ini" "Field 4" "State"
    ${unless} ${FileExists} "$R0\${BIN_${COMPONENT}}"
      MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_EXTERNAL_${COMPONENT}_NOTFOUND)
      Abort
    ${endif}
    StrCpy $Path${component} $R0
  ${endif}

  Pop $R1
  Pop $R0

!macroend

#--------------------------------
#Sections

Section -LaTeX ExternalLaTeX
  !insertmacro ExternalComponent LaTeX
SectionEnd

Section -ImageMagick ExternalImageMagick
  !insertmacro ExternalComponent ImageMagick
SectionEnd

Section -Ghostscript ExternalGhostscript
  !insertmacro ExternalComponent Ghostscript
SectionEnd

#--------------------------------
#Functions

Function InitSizeExternal

  #Get sizes of external component installers
  
  SectionGetSize ${ExternalLaTeX} $SizeLaTeX
  SectionGetSize ${ExternalImageMagick} $SizeImageMagick
  SectionGetSize ${ExternalGhostscript} $SizeGhostscript
  
  !ifndef SETUPTYPE_BUNDLE
    #Add download size
    IntOp $SizeLaTeX $SizeLaTeX + ${SIZE_DOWNLOAD_LATEX}
    IntOp $SizeImagemagick $SizeImagemagick + ${SIZE_DOWNLOAD_IMAGEMAGICK}
    IntOp $SizeGhostscript $SizeGhostscript + ${SIZE_DOWNLOAD_GHOSTSCRIPT}
  !endif
  
FunctionEnd

#--------------------------------
#Page functions

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
