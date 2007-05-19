Function UpdateModifiedFiles

 # list with modified files for LyX 1.5pre1 xx-05-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\TOC.lyx" # must be here when another \doc file has been changed
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\TOC.lyx"
 SetOutPath "$INSTDIR\Resources\doc\he"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\he\TOC.lyx"
 SetOutPath "$INSTDIR\Resources\examples\he"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\he\example_lyxified.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\he\example_raw.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\he\splash.lyx"

FunctionEnd

