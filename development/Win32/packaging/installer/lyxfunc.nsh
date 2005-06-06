!ifndef _LYX_FUNC_NSH_
!define _LYX_FUNC_NSH_

!macro LYX_DEFFUNC Name
  !define `${Name}` `!insertmacro LYX_FUNCTION_${Name}`
!macroend

!macro LYX_FUNC ShortName
  !undef `${ShortName}`
  !define `${ShortName}` `!insertmacro LYX_FUNCTION_${ShortName}_Call`
  Function `${ShortName}`
!macroend

!endif ; _LYX_FUNC_NSH_
