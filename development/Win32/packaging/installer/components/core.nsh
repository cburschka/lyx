/*

LyX Core
Binaries, resources, MSYS/Python, Aspell

*/

;--------------------------------
;Sections

Section -Core SecCore

  ;Install and register the core LyX files

  InitPluginsDir
  
  ;Remove previous local MSYS installation
  
  RMDir /r "$INSTDIR\shell"
  
  ;Remove previous local Python files
  
  RMDir /r "$INSTDIR\python\DLLs"
  RMDir /r "$INSTDIR\python\libs"
  RMDir /r "$INSTDIR\python\python25.dll"
  
  ;Remove previous latextools folder (moved to bin)
  
  RMDir /r "$INSTDIR\latextools"
  
  ;Binaries
  
  SetOutPath "$INSTDIR\bin"
  !insertmacro FileListLyXBin File "${FILES_LYX}\bin\"
  !insertmacro FileListQtBin File "${FILES_QT}\bin\"
  
  !ifdef BUILD_MSVC
    !insertmacro FileListDllMinGWBin Delete "$INSTDIR\bin\"
    !insertmacro FileListMinGWBin Delete "$INSTDIR\bin\"
    
    !insertmacro FileListDllMSVCBin File "${FILES_DEPS}\bin\"
    !insertmacro FileListMSVCBin File "${FILES_MSVC}\"
    !insertmacro FileListMSVCManifest File "..\"    
  !else
    !insertmacro FileListDllMSVCBin Delete "$INSTDIR\bin\"
    !insertmacro FileListMSVCBin Delete "$INSTDIR\bin\"  
    
    !insertmacro FileListDllMinGWBin File "${FILES_DEPS}\bin\"
    !insertmacro FileListMinGWBin File "${FILES_MINGW}\bin\" 
  !endif

  !insertmacro FileListNetpbmBin File "${FILES_NETPBM}\"
  !insertmacro FileListDvipostBin File "${FILES_DVIPOST}\"
  !insertmacro FileListDTLBin File "${FILES_DTL}\"

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
  !insertmacro FileListPythonDll File "${FILES_WINDOWS}\System32\"
  SetOutPath "$INSTDIR\python\Lib"
  !insertmacro FileListPythonLib File "${FILES_PYTHON}\Lib\"
  SetOutPath "$INSTDIR\python\Lib\encodings"
  !insertmacro FileListPythonLibEncodings File "${FILES_PYTHON}\Lib\encodings\"
  
  ;Aspell

  File /oname=$PLUGINSDIR\aspelldata.exe "${FILES_ASPELLDATA}\aspelldata-sep06.exe"

  ${if} $AllUsersInstall == ${TRUE}
    ExecWait '"$PLUGINSDIR\aspelldata.exe" /S /AllUsers'
  ${else}
    ExecWait '"$PLUGINSDIR\aspelldata.exe" /S'
  ${endif}

  Delete "$PLUGINSDIR\aspelldata.exe"

  ;Aiksarus data
  
  SetOutPath "$INSTDIR\aiksaurus"
  !insertmacro FileListAiksaurusData File "${FILES_AIKSAURUS}\"

SectionEnd
