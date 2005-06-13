!ifndef _STRTRIM_NSH_
!define _STRTRIM_NSH_

!include "lyxfunc.nsh"

!insertmacro LYX_DEFFUNC `StrLTrim`
!insertmacro LYX_DEFFUNC `StrRTrim`
!insertmacro LYX_DEFFUNC `StrTrim`
!insertmacro LYX_DEFFUNC `StrRTrimChar`

!macro LYX_FUNCTION_StrLTrim
  !insertmacro LYX_FUNC `StrLTrim`

    ; After this point:
    ; $0 = String (input)
    ; $1 = Temp (temp)

    ; Get input from user
    Exch $0
    Push $1

Loop:
    StrCpy $1 "$0" 1
    StrCmp "$1" " " TrimLeft
    StrCmp "$1" "$\r" TrimLeft
    StrCmp "$1" "$\n" TrimLeft
    StrCmp "$1" "	" TrimLeft ; this is a tab.
    GoTo Done
TrimLeft:
    StrCpy $0 "$0" "" 1
    Goto Loop

Done:
    Pop $1
    Exch $0
  FunctionEnd
!macroend

!macro LYX_FUNCTION_StrLTrim_Call Output Input
  Push `${Input}`
  Call StrLTrim
  Pop `${Output}`
!macroend

!macro LYX_FUNCTION_StrRTrim
  !insertmacro LYX_FUNC `StrRTrim`

    ; After this point:
    ; $0 = String (input)
    ; $1 = Temp (temp)

    ; Get input from user
    Exch $0
    Push $1

Loop:
    StrCpy $1 "$0" 1 -1
    StrCmp "$1" " " TrimRight
    StrCmp "$1" "$\r" TrimRight
    StrCmp "$1" "$\n" TrimRight
    StrCmp "$1" "	" TrimRight ; this is a tab
    GoTo Done
TrimRight:
    StrCpy $0 "$0" -1
    Goto Loop

Done:
    Pop $1
    Exch $0
  FunctionEnd
!macroend

!macro LYX_FUNCTION_StrRTrim_Call Output Input
  Push `${Input}`
  Call StrRTrim
  Pop `${Output}`
!macroend

!macro LYX_FUNCTION_StrTrim
  !insertmacro LYX_FUNC `StrTrim`
    Call StrLTrim
    Call StrRTrim
  FunctionEnd
!macroend

!macro LYX_FUNCTION_StrTrim_Call Output Input
  Push `${Input}`
  Call StrTrim
  Pop `${Output}`
!macroend

!macro LYX_FUNCTION_StrRTrimChar
  !insertmacro LYX_FUNC `StrRTrimChar`

    ; After this point:
    ; $0 = Input
    ; $1 = Char
    ; $2 = Temp

    ; Get input from user
    Exch $0
    Exch
    Exch $1
    Push $2

    StrCpy $2 "$0" 1 -1

    StrCmp "$2" "$1" TrimRight
    GoTo Done
TrimRight:
    StrCpy $0 "$0" -1

Done:
    Pop $2
    Pop $1
    Exch $0
  FunctionEnd
!macroend

!macro LYX_FUNCTION_StrRTrimChar_Call Output Input Char
  Push `${Char}`
  Push `${Input}`
  Call StrRTrimChar
  Pop `${Output}`
!macroend

!endif ; _STRTRIM_NSH_
