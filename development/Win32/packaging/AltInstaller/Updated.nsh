Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.8
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\elyxer.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.css" # always here
 File "${PRODUCT_SOURCEDIR}\bin\setup.py" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\DummyDocument1.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\DummyDocument2.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\DummyTextDocument.txt"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\eu"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\eu\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\hu"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\hu\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\it"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\it\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\nl"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\nl\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\pl"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\pl\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\pt"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\pt\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\sk"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\sk\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\sl"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\sl\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\doc\sv"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\sv\Tutorial.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\linguistics.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\linguistics.lyx"
 SetOutPath "$INSTDIR\Resources\images\math"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\dddot.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\ddddot.png"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\agutex.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\g-brief2.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\memoir.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\scrclass.inc"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\man\man1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\lyx.1"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\agutex.lyx"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

