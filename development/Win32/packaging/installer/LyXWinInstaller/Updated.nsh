Function UpdateModifiedFiles

 # list with modified files for LyX 1.5svn xx-05-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx.in"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\TOC.lyx" # must be here when another \doc file has been changed
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\armenian-article.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\TabellenBeispiel.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\simplecv.layout"

FunctionEnd

