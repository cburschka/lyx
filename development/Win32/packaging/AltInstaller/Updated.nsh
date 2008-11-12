Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.1
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsbook.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_6.py"
 SetOutPath "$INSTDIR\Resources\man\man1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\lyx.1"

FunctionEnd

