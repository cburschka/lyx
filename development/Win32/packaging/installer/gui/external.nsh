/*

external.nsh

Installation of external components: MiKTeX, ImageMagick, Ghostscript
Either an existing installation can be used or a the components can be
downloaded or installed (depending on the type of installer)

*/

Var ExternalPage
Var ExternalPage.Info
Var ExternalPage.Setup
Var ExternalPage.Existing
Var ExternalPage.Folder
Var ExternalPage.Folder_Text
Var ExternalPage.FolderBrowse
Var ExternalPage.FolderInfo
Var ExternalPage.NoInstall

Var ExternalPage.RadioButton.Click
Var ExternalPage.FolderBrowse.Click

Var LaTeXState
Var LaTeXSetup.State
Var LaTeXExisting.State
Var LaTeXNoInstall.State

Var ImageMagickState
Var ImageMagickSetup.State
Var ImageMagickExisting.State
Var ImageMagickNoInstall.State

Var GhostscriptState
Var GhostscriptSetup.State
Var GhostscriptExisting.State
Var GhostscriptNoInstall.State

Var ControlState
Var ComponentSize

!macro EXTERNAL_SHOWDIALOG COMPONENT

  # Build the dialog

  !insertmacro MUI_HEADER_TEXT $(TEXT_EXTERNAL_${COMPONENT}_TITLE) $(TEXT_EXTERNAL_${COMPONENT}_SUBTITLE)
  
  GetFunctionAddress $ExternalPage.RadioButton.Click ExternalRadioButtonClick
  GetFunctionAddress $ExternalPage.FolderBrowse.Click ExternalFolderBrowseClick

  nsDialogs::Create /NOUNLOAD 1018
  Pop $ExternalPage

  ${NSD_CreateLabel} 0u 0u 300u 20u $(TEXT_EXTERNAL_${COMPONENT}_INFO_${SETUPTYPE_NAME})
  Pop $ExternalPage.Info

  ${NSD_CreateRadioButton} 0u 40u 300u 10u $(TEXT_EXTERNAL_${COMPONENT}_${SETUPTYPE_NAME})
  Pop $ExternalPage.Setup
  nsDialogs::OnClick /NOUNLOAD $ExternalPage.Setup $ExternalPage.RadioButton.Click
  
  ${NSD_CreateRadioButton} 0u 60u 300u 10u $(TEXT_EXTERNAL_${COMPONENT}_FOLDER)
  Pop $ExternalPage.Existing
  nsDialogs::OnClick /NOUNLOAD $ExternalPage.Existing $ExternalPage.RadioButton.Click
  
  ${NSD_CreateText} 10u 80u 255u 12u $Path${COMPONENT}
  Pop $ExternalPage.Folder
 
  ${NSD_CreateBrowseButton} 270u 80u 20u 12u ...
  Pop $ExternalPage.FolderBrowse
  nsDialogs::OnClick /NOUNLOAD $ExternalPage.FolderBrowse $ExternalPage.FolderBrowse.Click
  
  ${NSD_CreateLabel} 10u 100u 290u 10u $(TEXT_EXTERNAL_${COMPONENT}_FOLDER_INFO)
  Pop $ExternalPage.FolderInfo
  
  ${NSD_CreateRadioButton} 0u 120u 300u 10u $(TEXT_EXTERNAL_${COMPONENT}_NONE)
  Pop $ExternalPage.NoInstall
  nsDialogs::OnClick /NOUNLOAD $ExternalPage.NoInstall $ExternalPage.RadioButton.Click
  
  !if ${COMPONENT} != LaTeX
  # ImageMagick and Ghostscript require Administrator or
  # power user privileges for installation
  ${If} $MultiUser.Privileges != "Admin"
  ${AndIf} $MultiUser.Privileges != "Power"
    EnableWindow $ExternalPage.Setup 0
  ${EndIf}
  !endif
  
  # Set the state of the controls to the previous user selection (or the default)

  ${If} $${COMPONENT}State == ""
    # Page is displayed for the first time, set the default
    ${If} $Path${COMPONENT} == ""
      !if ${COMPONENT} != LaTeX
      # ImageMagick and Ghostscript require Administrator or
      # power user privileges for installation
      # Setup won't be checked because it's disabled
      ${If} $MultiUser.Privileges != "User"
      ${AndIf} $MultiUser.Privileges != "Guest"
      !endif 
      
      ${NSD_SetState} $ExternalPage.Setup ${BST_CHECKED}
      
      !if ${COMPONENT} != LaTeX
      ${Else}
        ${NSD_SetState} $ExternalPage.NoInstall ${BST_CHECKED}
      ${EndIf}
      !endif
    ${Else}
      ${NSD_SetState} $ExternalPage.Existing ${BST_CHECKED}
    ${EndIf}
  ${Else} 
    ${NSD_SetState} $ExternalPage.Setup $${COMPONENT}Setup.State
    ${NSD_SetState} $ExternalPage.Existing $${COMPONENT}Existing.State
    ${NSD_SetState} $ExternalPage.NoInstall $${COMPONENT}NoInstall.State
  ${EndIf}
  
  
  Call ExternalRadioButtonClick
  
  nsDialogs::Show

!macroend

!macro EXTERNAL_VALIDATEDIALOG COMPONENT

  # Save state of controls
  StrCpy $${COMPONENT}State saved
  ${NSD_GetState} $ExternalPage.Setup $${COMPONENT}Setup.State
  ${NSD_GetState} $ExternalPage.Setup $${COMPONENT}Setup.State
  ${NSD_GetState} $ExternalPage.Existing $${COMPONENT}Existing.State
  ${NSD_GetState} $ExternalPage.NoInstall $${COMPONENT}NoInstall.State
  
  ${If} $${COMPONENT}Setup.State = ${BST_CHECKED}
    
    StrCpy $Setup${COMPONENT} ${TRUE}
    
    # Download or install, so we need hard drive space
    
    # Add size of component installer
    StrCpy $ComponentSize $Size${COMPONENT}
    
    ${If} $Path${COMPONENT} == ""
      # Not yet installed, add size of component itself
      IntOp $ComponentSize $ComponentSize + ${SIZE_${COMPONENT}}
    ${EndIf}
    
    SectionSetSize ${External${COMPONENT}} $ComponentSize
    
  ${Else}
    
    # No setup
    
    StrCpy $Setup${COMPONENT} ${FALSE}
    SectionSetSize ${External${COMPONENT}} 0
  
  ${EndIf}
  
  ${If} $${COMPONENT}Existing.State = ${BST_CHECKED}
  
    # Update location of component
    
    ${NSD_GetText} $ExternalPage.Folder $ExternalPage.Folder_Text
    
    # Verify whether the path exists
    
    ${IfNot} ${FileExists} "$ExternalPage.Folder_Text\${BIN_${COMPONENT}}"
      MessageBox MB_OK|MB_ICONEXCLAMATION $(TEXT_EXTERNAL_${COMPONENT}_NOTFOUND)
      Abort # Return to allow the user to correct the location
    ${EndIf}
    
    StrCpy $Path${COMPONENT} $ExternalPage.Folder_Text
  
  ${EndIf}
  
!macroend

Function ExternalRadioButtonClick

  # Enable the folder selection controls depending
  # if the option to use an existing installation is selected

  ${NSD_GetState} $ExternalPage.Existing $ControlState
  
  ${If} $ControlState = ${BST_CHECKED}
    EnableWindow $ExternalPage.Folder 1
    EnableWindow $ExternalPage.FolderBrowse 1
    EnableWindow $ExternalPage.FolderInfo 1
  ${Else}
    EnableWindow $ExternalPage.Folder 0
    EnableWindow $ExternalPage.FolderBrowse 0
    EnableWindow $ExternalPage.FolderInfo 0
  ${EndIf}

FunctionEnd

Function ExternalFolderBrowseClick

  # Browse button clicked
  
  # Get current folder to set as default
  ${NSD_GetText} $ExternalPage.Folder $ExternalPage.Folder_Text
  
  # Browse for new folder
  nsDialogs::SelectFolderDialog /NOUNLOAD "" $ExternalPage.Folder_Text
  Pop $ExternalPage.Folder_Text
  ${NSD_SetText} $ExternalPage.Folder $ExternalPage.Folder_Text

FunctionEnd

# Page functions

Function PageExternalLaTeX
  !insertmacro EXTERNAL_SHOWDIALOG LaTeX
FunctionEnd

Function PageExternalLaTeXValidate
  !insertmacro EXTERNAL_VALIDATEDIALOG LaTeX
FunctionEnd

Function PageExternalImageMagick
  !insertmacro EXTERNAL_SHOWDIALOG ImageMagick
FunctionEnd

Function PageExternalImageMagickValidate
  !insertmacro EXTERNAL_VALIDATEDIALOG ImageMagick
FunctionEnd

Function PageExternalGhostscript
  !insertmacro EXTERNAL_SHOWDIALOG Ghostscript
FunctionEnd

Function PageExternalGhostscriptValidate
  !insertmacro EXTERNAL_VALIDATEDIALOG Ghostscript
FunctionEnd
