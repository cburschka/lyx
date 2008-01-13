Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.4
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"  
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_4.py"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

