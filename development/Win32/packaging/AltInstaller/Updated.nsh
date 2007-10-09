Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.3svn
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\emacs.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\mac.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\xemacs.bind" 
 SetOutPath "$INSTDIR\Resources\fonts"
 File "${PRODUCT_SOURCEDIR}\Resources\fonts\esint10.ttf"
 File "${PRODUCT_SOURCEDIR}\Resources\fonts\wasy10.ttf"
 SetOutPath "$INSTDIR\Resources\layouts"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\templates"
 SetOutPath "$INSTDIR\Resources\ui"

FunctionEnd

