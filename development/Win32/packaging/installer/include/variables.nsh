/*

variables.nsh

Variables that are shared between multiple files

*/

Var PathLaTeX
Var PathLaTeXLocal
Var PathBibTeXEditor

#Var SetupLaTeX
#Var SizeLaTeX

Var Acrobat
Var APPDATemp
Var AppPre
var AppSubfolder
Var AppSuff
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
Var ImageEditorPath
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
Var ThesCode
Var ThesCodes
Var UpdateFNDBReturn
Var UserList

Var LangName

Var PrinterConf
