# this file contains the installation actions for the third-party programs

Function Ghostscript

  # if GhostScript is not installed
  ${if} $GhostscriptPath == ""
   # register Ghostscript
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion}" "GS_DLL" "${GhostscriptDir}\bin\gsdll32.dll"
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript\${GhostscriptVersion}" "GS_LIB" "${GhostscriptDir}\lib"
   
   WriteRegStr HKLM "SOFTWARE\GPL Ghostscript" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
   StrCpy $GhostscriptPath "${GhostscriptDir}\bin"
  ${else}
   # delete unnecessary files
   RMDir /r ${GhostscriptDir}
   StrCpy $DelGSDir "Yes" # used later in function ImageMagick
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
   # delete the "etc" folder when also the Ghostscript folder was deleted because it is then empty 
   ${if} $DelGSDir == "Yes"
    RMDir /r "$INSTDIR\etc"
   ${endif}
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

# -------------------------------------------

!if ${INSTALLER_VERSION} == "Complete"

 Function PostScript

  # if no PostScript viewer is installed
  ${if} $PSVPath == ""
   ${if} $InstallGSview == "true"
    # launch installer
    MessageBox MB_OK|MB_ICONINFORMATION "$(GSviewInfo)"
    ExecWait "$INSTDIR\${GSviewInstall}"
    # test if GSview is installed
    StrCpy $PSVPath ""
    ReadRegStr $PSVPath HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\gsview32.exe" "Path"    
    ${if} $PSVPath == ""
     MessageBox MB_OK|MB_ICONEXCLAMATION "$(GSviewError)"
    ${endif}
   ${endif}
  ${endif}

 FunctionEnd

!endif # endif ${INSTALLER_VERSION} == "Complete"

# -------------------------------------------

!if ${INSTALLER_VERSION} == "Complete"

 Function BibTeX

  # if no BibTeX editor is installed
  ${if} $BibTeXEditorPath == ""
   ${if} $InstallJabRef == "true"
    # launch installer
    MessageBox MB_OK|MB_ICONINFORMATION "$(JabRefInfo)"
    ExecWait "$INSTDIR\${JabRefInstall}"
    # test if JabRef is installed
    StrCpy $BibTeXEditorPath ""
    ReadRegStr $BibTeXEditorPath HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "UninstallString"    
    ${if} $BibTeXEditorPath == ""
     MessageBox MB_OK|MB_ICONEXCLAMATION "$(JabRefError)"
    ${else}
     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\JabRef ${JabRefVersion}" "OnlyWithLyX" "Yes${PRODUCT_VERSION_SHORT}" # special entry to tell the uninstaller that it was installed with LyX
    ${endif}
   ${endif}
  ${endif}

 FunctionEnd

!endif # endif ${INSTALLER_VERSION} == "Complete"

