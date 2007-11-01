Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.3
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\emacs.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\mac.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\xemacs.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de\clipart"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\clipart\ERT.png"
 SetOutPath "$INSTDIR\Resources\examples\es"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\es\ItemizeBullets.lyx"
 SetOutPath "$INSTDIR\Resources\fonts"
 File "${PRODUCT_SOURCEDIR}\Resources\fonts\esint10.ttf"
 File "${PRODUCT_SOURCEDIR}\Resources\fonts\wasy10.ttf"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsmaths.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsmaths-seq.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\beamer.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\foils.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\powerdot.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\slides.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\templates"
 SetOutPath "$INSTDIR\Resources\ui"

FunctionEnd

