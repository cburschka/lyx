Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.7
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 #File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 #File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 #File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\ca"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\ca\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\he"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\he\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\examples\ca"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ca\ItemizeBullets.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ca\mathed.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ca\splash.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 #File "${PRODUCT_SOURCEDIR}\Resources\layouts\aa.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\scripts"
 #File "${PRODUCT_SOURCEDIR}\Resources\scripts\csv2lyx.py"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\hollywood.lyx"
 SetOutPath "$INSTDIR\Resources\man\man1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\lyx.1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\tex2lyx.1"

FunctionEnd

