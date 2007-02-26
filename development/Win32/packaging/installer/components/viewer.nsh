/*

Option to download GSView (PDF/Postscript viewer)

*/

#--------------------------------
#Sections

Section -Viewer ExternalViewer
  !insertmacro ExternalComponent Viewer
SectionEnd

#--------------------------------
#Functions

Function InitSizeViewer

  #Get size of viewer installer
 
  SectionGetSize ${ExternalViewer} $SizeViewer
  
  !ifndef SETUPTYPE_BUNDLE
    #Add download size
    IntOp $SizeViewer $SizeViewer + ${SIZE_DOWNLOAD_VIEWER}
  !endif
  
FunctionEnd

#--------------------------------
#Page functions

Function PageViewer

  #Show page if no viewer is installed, the user has Power User or Administrator priviledges and
  #Ghostscript is installed or will be installed

  ${if} $AdminOrPowerUser == ${FALSE}
  ${orif} $PathViewer == "associated"
    Abort
  ${endif}

  ${if} $PathGhostscript == ""
  ${andif} $SetupGhostscript == ${FALSE}
    !insertmacro SetComponentState $R0 Viewer
    Abort
  ${endif}

  !insertmacro MUI_HEADER_TEXT $(TEXT_VIEWER_TITLE) $(TEXT_VIEWER_SUBTITLE)
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "viewer.ini"

FunctionEnd

Function PageViewerValidate

  #Download?
  !insertmacro MUI_INSTALLOPTIONS_READ $R0 "viewer.ini" "Field 2" "State"
  !insertmacro SetComponentState $R0 Viewer

FunctionEnd
