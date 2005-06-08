; File download.nsh
; This file is part of LyX, the document processor.
; http://www.lyx.org/
; Licence details can be found in the file COPYING or copy at
; http://www.lyx.org/about/license.php3

; Author Angus Leeming
; Full author contact details are available in file CREDITS or copy at
; http://www.lyx.org/about/credits.php

!ifndef _DOWNLOAD_NSH_
!define _DOWNLOAD_NSH_

!include "lyxfunc.nsh"
!include "LogicLib.nsh"

!insertmacro LYX_DEFFUNC `ReadDownloadValues`
!insertmacro LYX_DEFFUNC `EnableBrowseControls`
!insertmacro LYX_DEFFUNC `DownloadEnter`
!insertmacro LYX_DEFFUNC `DownloadLeave`


!macro LYX_FUNCTION_ReadDownloadValues
  !insertmacro LYX_FUNC `ReadDownloadValues`

    ; The stack contains:
    ; TOP
    ; FolderPath
    ; SelectFolder
    ; Download
    ; DoNothing

    ; After this point:
    ; $0 = FolderPath
    ; $1 = SelectFolder
    ; $2 = Download
    ; $3 = DoNothing
    ; $4 = temp

    Exch $0
    Exch
    Exch $1
    Exch 2
    Exch $2
    Exch 3
    Exch $3

    ; Populate the registers with the values in the widgets.

    ; DoNothing.
    ; If the widget is disabled then set DoNothing ($3) to 0.
    ; Otherwise, set it equal to the "state" variable of the field.
    !insertmacro MUI_INSTALLOPTIONS_READ $3 "ioDownload.ini" "Field 2" "Flags"
    IntOp $3 $3 & DISABLED
    ${if} $3 == 1
      StrCpy $3 0
    ${else}
      !insertmacro MUI_INSTALLOPTIONS_READ $3 "ioDownload.ini" "Field 2" "State"
    ${endif}

    ; Download
    !insertmacro MUI_INSTALLOPTIONS_READ $2 "ioDownload.ini" "Field 3" "State"
    ; SelectFolder
    !insertmacro MUI_INSTALLOPTIONS_READ $1 "ioDownload.ini" "Field 4" "State"
    ; FolderPath
    !insertmacro MUI_INSTALLOPTIONS_READ $0 "ioDownload.ini" "Field 5" "State"

    ; Return output to user.
    ; The stack available to the user contains:
    ; TOP
    ; Modified FolderPath
    ; Modified SelectFolder
    ; Modified Download
    ; Modified DoNothing

    ; $0 = FolderPath
    ; $1 = SelectFolder
    ; $2 = Download
    ; $3 = DoNothing

    Exch $3
    Exch 3
    Exch $2
    Exch 2
    Exch $1
    Exch
    Exch $0
  FunctionEnd
!macroend


!macro LYX_FUNCTION_EnableBrowseControls
  !insertmacro LYX_FUNC `EnableBrowseControls`

    ; After this point:
    ; $0 = SelectFolder
    ; $1 = temp
    ; $2 = temp
    ; $3 = temp

    Push $0
    Push $1
    Push $2
    Push $3

    ; Populate the registers with the values in the widgets.
    ; We're interested only in $0 (SelectFolder) here.
    ${ReadDownloadValues} $1 $2 $0 $3

    ; Get the dialog HWND, storing it in $1
    FindWindow $1 "#32770" "" $HWNDPARENT

    ; To get the HWND of the controls use:
    ; GetDlgItem (output var)
    ; (hwnd of the custom dialog) (1200 + Field number - 1)

    ; Get the Browse textbox ID
    GetDlgItem $2 $1 1204
    ; Enable it if the SelectFolder ($0) checkbox is selected.
    EnableWindow $2 $0

    ; get Browse button ID
    GetDlgItem $2 $1 1205
    ; Enable it if the Folder checkbox is selected.
    EnableWindow $2 $0

    ; Remove temporaries from stack.
    Pop $3
    Pop $2
    Pop $1
    Pop $0
  FunctionEnd
!macroend


!macro DownloadEnter_Private ExePath RegistryKey RegistrySubKey RemoveFromPath AddtoPath Required DownloadLabel HomeLabel PageHeader PageDescription
 !define skipBackupLbl "skipBackup_${__LINE__}"

  StrCpy ${ExePath} ""
  ReadRegStr ${ExePath} HKLM "${RegistryKey}" "${RegistrySubKey}"

  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 1" "Text" ""
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 2" "Text" "$(DownloadPageField2)"

  Push $0
  ${if} ${Required} == 1
    StrCpy $0 "NOTIFY"
  ${else}
    StrCpy $0 "DISABLED"
  ${endif}
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 2" "Flags" $0
  Pop $0

  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 2" "State" "0"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 3" "Text" "${DownloadLabel}"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 4" "Text" "${HomeLabel}"

  ${if} ${ExePath} == ""
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 3" "State" "1"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 4" "State" "0"
    !insertmacro MUI_INSTALLOPTIONS_READ $0 "ioDownload.ini" "Field 5" "Flags"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 5" "Flags" $0|DISABLED
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 5" "State" ""
  ${else}
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 3" "State" "0"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 4" "State" "1"

    ${StrRep} "${ExePath}" "${ExePath}" "${RemoveFromPath}" ""
    StrCpy ${ExePath} "${ExePath}${AddtoPath}"
    !insertmacro MUI_INSTALLOPTIONS_WRITE "ioDownload.ini" "Field 5" "State" "${ExePath}"
  ${endif}

  ClearErrors

  !insertmacro MUI_HEADER_TEXT "${PageHeader}" "${PageDescription}"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "ioDownload.ini"
 !undef skipBackupLbl
!macroend


!macro LYX_FUNCTION_DownloadEnter
  !insertmacro LYX_FUNC `DownloadEnter`

    ; The stack contains:
    ; TOP
    ; ExePath
    ; RegistryKey
    ; RegistrySubKey
    ; RemoveFromPath
    ; AddtoPath
    ; Required
    ; DownloadLabel
    ; HomeLabel
    ; PageHeader
    ; PageDescription

    ; After this point:
    ; $0 = ExePath
    ; $1 = RegistryKey
    ; $2 = RegistrySubKey
    ; $3 = RemoveFromPath
    ; $4 = AddtoPath
    ; $5 = Required
    ; $6 = DownloadLabel
    ; $7 = HomeLabel
    ; $8 = PageHeader
    ; $9 = PageDescription

    Exch $0
    Exch
    Exch $1
    Exch 2
    Exch $2
    Exch 3
    Exch $3
    Exch 4
    Exch $4
    Exch 5
    Exch $5
    Exch 6
    Exch $6
    Exch 7
    Exch $7
    Exch 8
    Exch $8
    Exch 9
    Exch $9

    ; Use a macro simply to make life understandable.
    !insertmacro DownloadEnter_Private "$0" "$1" "$2" "$3" "$4" "$5" "$6" "$7" "$8" "$9"

    ; Return output to user.
    Exch $9
    Exch 9
    Exch $8
    Exch 8
    Exch $7
    Exch 7
    Exch $6
    Exch 6
    Exch $5
    Exch 5
    Exch $4
    Exch 4
    Exch $3
    Exch 3
    Exch $2
    Exch 2
    Exch $1
    Exch
    Exch $0
  FunctionEnd
!macroend


!macro DownloadLeave_Private DoNotRequire Download FolderPath URL EnterFolder ExeName InvalidFolder
 !define skipBackupLbl "skipBackup_${__LINE__}"
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "ioDownload.ini" "Settings" "State"

  StrCmp $0 0 go_on  ; Next button?

  ${EnableBrowseControls}
  Abort ; Return to the page

go_on:
  ${ReadDownloadValues} ${DoNotRequire} ${Download} $0 ${FolderPath}

  ${if} ${DoNotRequire} == 1
    ;
  ${elseif} ${Download} == 1
    StrCpy ${FolderPath} ""
    ExecShell open "${URL}"
  ${else}
    ${StrTrim} ${FolderPath}

    ${if} ${FolderPath} == ""
      MessageBox MB_OK "${EnterFolder}"
      Abort
    ${endif}

    ${if} ${FileExists} "${FolderPath}"
      ${StrRep} ${FolderPath} ${FolderPath} "${ExeName}" ""

      ${if} ${FileExists} "${FolderPath}${ExeName}"
      ${else}
        MessageBox MB_OK "${InvalidFolder}"
        Abort
      ${endif}
    ${else}
      MessageBox MB_OK "${InvalidFolder}"
      Abort
    ${endif}
  ${endif}
 !undef skipBackupLbl
!macroend


!macro LYX_FUNCTION_DownloadLeave
  !insertmacro LYX_FUNC `DownloadLeave`

    ; The stack contains:
    ; TOP
    ; DoNotRequire
    ; Download
    ; FolderPath
    ; URL
    ; EnterFolder
    ; ExeName
    ; InvalidFolder

    ; After this point:
    ; $0 = DoNotRequire
    ; $1 = Download
    ; $2 = FolderPath
    ; $3 = URL
    ; $4 = EnterFolder
    ; $5 = ExeName
    ; $6 = InvalidFolder

    Exch $0
    Exch
    Exch $1
    Exch 2
    Exch $2
    Exch 3
    Exch $3
    Exch 4
    Exch $4
    Exch 5
    Exch $5
    Exch 6
    Exch $6

    ; Use a macro simply to make life understandable.
    !insertmacro DownloadLeave_Private "$0" "$1" "$2" "$3" "$4" "$5" "$6"

    ; Return output to user.
    Exch $6
    Exch 6
    Exch $5
    Exch 5
    Exch $4
    Exch 4
    Exch $3
    Exch 3
    Exch $2
    Exch 2
    Exch $1
    Exch
    Exch $0
  FunctionEnd
!macroend


!macro LYX_FUNCTION_ReadDownloadValues_Call DoNothing Download SelectFolder FolderPath
  Push `${DoNothing}`
  Push `${Download}`
  Push `${SelectFolder}`
  Push `${FolderPath}`
  Call ReadDownloadValues
  Pop `${FolderPath}`
  Pop `${SelectFolder}`
  Pop `${Download}`
  Pop `${DoNothing}`
!macroend


!macro LYX_FUNCTION_EnableBrowseControls_Call
  Call EnableBrowseControls
!macroend


!macro LYX_FUNCTION_DownloadEnter_Call ExePath RegistryKey RegistrySubKey RemoveFromPath AddtoPath Required DownloadLabel HomeLabel PageHeader PageDescription
  Push `${PageDescription}`
  Push `${PageHeader}`
  Push `${HomeLabel}`
  Push `${DownloadLabel}`
  Push `${Required}`
  Push `${AddtoPath}`
  Push `${RemoveFromPath}`
  Push `${RegistrySubKey}`
  Push `${RegistryKey}`
  Push `${ExePath}`
  Call DownloadEnter
  ; Empty the stack of all the stuff we've just added.
  ; We're not interested in keeping it, so just fill $0 repeatedly.
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
!macroend


!macro LYX_FUNCTION_DownloadLeave_Call DoNotRequire Download FolderPath URL EnterFolder ExeName InvalidFolder
  Push `${InvalidFolder}`
  Push `${ExeName}`
  Push `${EnterFolder}`
  Push `${URL}`
  Push `${FolderPath}`
  Push `${Download}`
  Push `${DoNotRequire}`
  Call DownloadLeave
  ; Empty the stack of all the stuff we've just added.
  Pop `${DoNotRequire}`
  Pop `${Download}`
  Pop `${FolderPath}`
  Pop `$0`
  Pop `$0`
  Pop `$0`
  Pop `$0`
!macroend

!endif ; _DOWNLOAD_NSH_
