/*

reinstall.nsh

Verify whether the LyX version of the installer is already installed.
If yes, ask the user whether the program files need to be reinstalled.

*/

Var ReinstallPage
Var ReinstallPage.Info
Var ReinstallPage.CheckBox
Var ReinstallPage.CheckBox.State

Var InstalledVersion

# Page functions

Function PageReinstall

  # Check whether this version is already installed
  # If so, some registry key like Software/Lyx16 should exist
  ReadRegStr $InstalledVersion SHELL_CONTEXT ${APP_REGKEY} "Version"

  ${If} $InstalledVersion != "${APP_VERSION}"
    # Different version, go ahead
    Abort
  ${EndIf}
  
  # Same version is already installed, ask the user

  !insertmacro MUI_HEADER_TEXT $(TEXT_REINSTALL_TITLE) $(TEXT_REINSTALL_SUBTITLE)
  
  # Build the dialog

  nsDialogs::Create /NOUNLOAD 1018
  Pop $ReinstallPage

  ${NSD_CreateLabel} 0u 0u 300u 60u $(TEXT_REINSTALL_INFO)
  Pop $ReinstallPage.Info

  ${NSD_CreateCheckBox} 20u 60u 280u 10u $(TEXT_REINSTALL_ENABLE)
  Pop $ReinstallPage.CheckBox
  
  ${If} ${SectionIsSelected} ${SecProgramFiles}
    ${NSD_SetState} $ReinstallPage.CheckBox ${BST_CHECKED}
  ${Else}
    ${NSD_SetState} $ReinstallPage.CheckBox ${BST_UNCHECKED}
  ${EndIf}

  nsDialogs::Show

FunctionEnd

Function PageReinstallValidate

  # Get state of checkbox
  ${NSD_GetState} $ReinstallPage.CheckBox $ReinstallPage.CheckBox.State
  
  # Set whether the programs files need to be installed
  ${If} $ReinstallPage.CheckBox.State = ${BST_CHECKED}
    !insertmacro SelectSection ${SecProgramFiles}
  ${Else}
    !insertmacro UnselectSection ${SecProgramFiles}
  ${EndIf}

FunctionEnd
