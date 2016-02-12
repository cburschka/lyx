/*

variables.nsh

Variables that are shared between multiple files

*/

Var PathLaTeX
Var PathLaTeXLocal
Var PathBibTeXEditor

Var APPDATemp
Var AppPre
var AppSubfolder
Var AppSuff
Var ComputerName
Var CreateDesktopIcon
Var CreateFileAssociations
Var DictCode
Var DictCodes
Var DelPythonFiles
Var EditorPath
Var FoundDict
Var FoundThes
Var GhostscriptPath
Var GnumericPath
Var ImageEditor
Var ImageEditorPath
Var Is64bit
!if ${SETUPTYPE} == BUNDLE
 Var InstallJabRef
!endif
Var JabRefInstalled
Var LaTeXName
Var LaTeXInstalled
Var LilyPondPath
Var MiKTeXUser
Var MiKTeXVersion
Var OldVersionNumber
Var PandocPath
Var Pointer
Var PSVPath
Var PythonPath
Var Search
!if ${SETUPTYPE} != BUNDLE
 Var State
!endif
Var SVGPath
Var StartmenuFolder
Var String
Var Tmp
Var ThesCode
Var ThesCodes
Var UpdateFNDBReturn
Var UserList
Var UserName

Var LangName
