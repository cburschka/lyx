Function UpdateModifiedFiles

 # list with modified files for LyX 1.6-svn-rxxxxx
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources"
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 SetOutPath "$INSTDIR\Resources\bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 File "${PRODUCT_SOURCEDIR}\Resources\bind\mac.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Userguide.lyx"
 #SetOutPath "$INSTDIR\Resources\layouts"
 #File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsmaths.inc"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\LyX.py"
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_6.py"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\classic.ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"

FunctionEnd

