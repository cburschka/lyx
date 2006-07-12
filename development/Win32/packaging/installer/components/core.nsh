/*

LyX Core
Binaries, resources, MSYS/Python, Aspell

*/

;--------------------------------
;Sections

Section -Core SecCore

  ;Install and register the core LyX files

  InitPluginsDir
  
  ;Binaries
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXBin File "${FILES_LYX}\bin\"
  !insertmacro FileListQtBin File "${FILES_QT}\bin\"
  !insertmacro FileListMinGWBin File "${FILES_MINGW}\bin\"
  !insertmacro FileListDllBin File "${FILES_DEPS}\bin\"

  ;LaTeX Tools
  SetOutPath "$INSTDIR\latextools"
  !insertmacro FileListNetpbmBin File "${FILES_DEPS}\bin\"
  !insertmacro FileListDvipostBin File "${FILES_DEPS}\bin\"
  !insertmacro FileListDTLBin File "${FILES_DEPS}\bin\"

  ;Icons
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXIcons File "${FILES_ICONS}\"

  ;Resources
  SetOutPath "$INSTDIR"
  File /r "${FILES_LYX}\Resources"
  SetOutPath "$INSTDIR\Resources\fonts"
  !insertmacro FileListFonts File "${FILES_FONTS}\"  
  
  ;Components of Python
  SetOutPath "$INSTDIR\python"
  !insertmacro FileListPythonBin File "${FILES_PYTHON}\"
  SetOutPath "$INSTDIR\python"
  !insertmacro FileListPythonDll File "${FILES_WINDOWS}\"
  SetOutPath "$INSTDIR\python\DLLs"
  !insertmacro FileListPythonDLLs File "${FILES_PYTHON}\DLLs\"
  SetOutPath "$INSTDIR\python\Lib"
  !insertmacro FileListPythonLib File "${FILES_PYTHON}\Lib\"
  SetOutPath "$INSTDIR\python\Lib\encodings"
  !insertmacro FileListPythonLibEncodings File "${FILES_PYTHON}\Lib\encodings\"
  SetOutPath "$INSTDIR\python\libs"
  !insertmacro FileListPythonLibs File "${FILES_PYTHON}\libs\"

  ;Components of MSYS
  SetOutPath "$INSTDIR\shell\bin"
  !insertmacro FileListMSYSBin File "${FILES_MSYS}\bin\"
  SetOutPath "$INSTDIR\shell\etc"
  !insertmacro FileListMSYSEtc File "${FILES_MSYS}\etc\"     

  ;Aspell

  File /oname=$PLUGINSDIR\AspellData-0.60.4.exe "${FILES_ASPELLDATA}\AspellData-0.60.4.exe"

  ${if} $AllUsersInstall == ${TRUE}
    ExecWait '"$PLUGINSDIR\aspelldata-0.60.4.exe" /S /AllUsers'
  ${else}
    ExecWait '"$PLUGINSDIR\aspelldata-0.60.4.exe" /S'
  ${endif}

  Delete "$PLUGINSDIR\aspelldata-0.60.4.exe"

  ;Aiksarus Data
  SetOutPath "$INSTDIR\aiksaurus"
  !insertmacro FileListAiksaurusData File "${FILES_DEPS}\share\Aiksaurus\"

  ;dvipost package if MiKTeX is installed

  Call SearchLaTeXLocalRoot
  Pop $R0

  ${if} $R0 != ""
    SetOutPath "$R0\tex\latex\dvipost"
    File "${FILES_DVIPOST}\dvipost.sty"
  ${endif}

SectionEnd
