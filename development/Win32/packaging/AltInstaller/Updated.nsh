Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.7
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\ca"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\ca\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\he"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\he\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\doc\uk"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\uk\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\examples\ca"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ca\ItemizeBullets.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ca\mathed.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ca\splash.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\beispiel_roh.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\splash.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\singlecol.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\svglobal3.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\svjour3.inc"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\scripts"
 File "${PRODUCT_SOURCEDIR}\Resources\scripts\convertDefault.py"
 File "${PRODUCT_SOURCEDIR}\Resources\scripts\lyxpreview2bitmap.py"
 File "${PRODUCT_SOURCEDIR}\Resources\scripts\lyxpreview_tools.py"
 File "${PRODUCT_SOURCEDIR}\Resources\scripts\legacy_lyxpreview2ppm.py"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\hollywood.lyx"
 SetOutPath "$INSTDIR\Resources\man\man1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\lyx.1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\tex2lyx.1"

FunctionEnd

