Function UpdateModifiedFiles

 ; list with modified files for LyX 1.5svn 30-04-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" ; always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" ; always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 File "${PRODUCT_SOURCEDIR}\Resources\languages"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" ; always here
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\sciword.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" ; must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\TOC.lyx" ; must be here when another \doc file has been changed
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\TOC.lyx" ; must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\it"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\it\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\it\TOC.lyx" ; must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\examples"
 Delete "$INSTDIR\Resources\examples\de\Dezimal.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\simplecv.lyx"
 SetOutPath "$INSTDIR\Resources\images"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math-superscript.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_set-rotate-cell.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_set-rotate-tabular.xpm"
 SetOutPath "$INSTDIR\Resources\images\math"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\super.xpm"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\simplecv.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\LyX.py"
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_5.py"
 SetOutPath "$INSTDIR\Resources\scripts"
 File "${PRODUCT_SOURCEDIR}\Resources\scripts\tex_copy.py"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

