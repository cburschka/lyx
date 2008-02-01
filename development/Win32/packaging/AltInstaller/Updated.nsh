Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.4
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc"
 File /r "${PRODUCT_SOURCEDIR}\Resources\bind"
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"  
 SetOutPath "$INSTDIR\Resources\examples\es"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\es\mathed.lyx"
 SetOutPath "$INSTDIR\Resources\examples\ja"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\ja\splash.lyx"
 SetOutPath "$INSTDIR\Resources\images"
 File "${PRODUCT_SOURCEDIR}\Resources\images\box-insert.xpm"
 File "${PRODUCT_SOURCEDIR}\Resources\images\buffer-update_pdf2.xpm"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\memoir.layout"  
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_4.py"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

