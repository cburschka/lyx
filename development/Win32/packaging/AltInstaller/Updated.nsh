Function UpdateModifiedFiles

 # list with modified files for LyX 1.5rc2 xx-06-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\layouts"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx.in"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\FAQ.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\clipart"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\clipart\escher-lsd.eps"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\clipart\mobius.eps"
 SetOutPath "$INSTDIR\Resources\examples\fa"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\fa\splash.lyx"
 SetOutPath "$INSTDIR\Resources\examples\hu"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\hu\example_lyxified.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\hu\example_raw.lyx"
 SetOutPath "$INSTDIR\Resources\images"
 File "${PRODUCT_SOURCEDIR}\Resources\images\closetab.xpm"
 SetOutPath "$INSTDIR\Resources\images\math"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\functions.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\varrho.xpm"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\IEEEtran.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\iop-article.lyx"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"

FunctionEnd

