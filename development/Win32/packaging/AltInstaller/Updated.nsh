Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.2
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\external_templates"
 #File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 #SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 #File "${PRODUCT_SOURCEDIR}\Resources\examples\achemso.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\hanging.module"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\minimalistic.module"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\siamltex.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\stdinsets.inc"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_6.py"
 SetOutPath "$INSTDIR\Resources\man"
 File "${PRODUCT_SOURCEDIR}\Resources\man\tex2lyx.1"

FunctionEnd

