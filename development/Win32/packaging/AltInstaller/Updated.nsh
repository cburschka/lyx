Function UpdateModifiedFiles

 # list with modified files for LyX 1.5pre1 xx-05-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\mac.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx.in"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\TOC.lyx" # must be here when another \doc file has been changed
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de\clipart"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\clipart\FussnoteQt4.png"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\TOC.lyx"
 SetOutPath "$INSTDIR\Resources\doc\he"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\he\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\he\TOC.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\powerdot-example.lyx"
 SetOutPath "$INSTDIR\Resources\examples\he"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\he\example_lyxified.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\he\example_raw.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\he\splash.lyx"
 SetOutPath "$INSTDIR\Resources\images"
 File "${PRODUCT_SOURCEDIR}\Resources\images\toolbar-toggle_table.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\toolbar-toggle_math_panels.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\toolbar-toggle_math.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-insert.xpm"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\beamer.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\powerdot.layout" 
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\default.ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

