Function UpdateModifiedFiles

 # list with modified files for LyX 1.5rc2 xx-06-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 SetOutPath "$INSTDIR\Resources"
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\TOC.lyx" # must be here when another \doc file has been changed
 SetOutPath "$INSTDIR\Resources\doc\clipart"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\clipart\escher-lsd.eps"  
 File "${PRODUCT_SOURCEDIR}\Resources\doc\clipart\mobius.eps"
 SetOutPath "$INSTDIR\Resources\examples"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\beamer.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\ijmpc.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\ijmpd.layout"

FunctionEnd

