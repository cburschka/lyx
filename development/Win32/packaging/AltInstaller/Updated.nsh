Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.2svn 22-09-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"  
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File /r "${PRODUCT_SOURCEDIR}\Resources\images"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc"
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc\biblio"
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc\clipart"
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc\de"
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc\es"
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc\es\biblio"
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc\es\clipart"
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\splash.lyx"
 SetOutPath "$INSTDIR\Resources\images\math"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\lparen_rparen.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\lbracket_rbracket.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\lbrace_rbrace.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\math\cases.xpm"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\beamer.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsmaths-seq.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsmaths-plain.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsbook.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsart-seq.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsart.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsmaths.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsdefs.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsart-plain.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\LyX.py"
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_lang.py"
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"

FunctionEnd

