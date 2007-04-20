Function UpdateModifiedFiles

 ; list with modified files for LyX 1.5svn xx-04-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" ; always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" ; always here
 SetOutPath "$INSTDIR\Resources"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" ; always here
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" ; must be here when another \doc file has been changed
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\TOC.lyx" ; must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\images"
 File /r "${PRODUCT_SOURCEDIR}\Resources\images\math"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\g-brief2.layout"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\default.ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\classic.ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

