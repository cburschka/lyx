!ifndef _STRTRIM_NSH_
!define _STRTRIM_NSH_

!include "lyxfunc.nsh"

!insertmacro LYX_DEFFUNC `StrTrim`

!macro LYX_FUNCTION_StrTrim
  !insertmacro LYX_FUNC `StrTrim`

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
    GoTo Loop2
TrimLeft:
    StrCpy $0 "$0" "" 1
    Goto Loop

Loop2:
    StrCpy $1 "$0" 1 -1
    StrCmp "$1" " " TrimRight
    StrCmp "$1" "$\r" TrimRight
    StrCmp "$1" "$\n" TrimRight
    StrCmp "$1" "	" TrimRight ; this is a tab
    GoTo Done
TrimRight:
    StrCpy $0 "$0" -1
    Goto Loop2

Done:
    Pop $1
    Exch $0
  FunctionEnd
!macroend

!macro LYX_FUNCTION_StrTrim_Call String
  Push `${String}`
  Call StrTrim
  Pop `${String}`
!macroend

!endif ; _STRTRIM_NSH_
