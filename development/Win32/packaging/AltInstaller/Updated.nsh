Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.8
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\elyxer.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.css" # always here
 #File "${PRODUCT_SOURCEDIR}\bin\QtCore4.dll"
 #File "${PRODUCT_SOURCEDIR}\bin\QtGui4.dll"
 File "${PRODUCT_SOURCEDIR}\bin\setup.py" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 #File "${PRODUCT_SOURCEDIR}\Resources\languages"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 #SetOutPath "$INSTDIR\Resources\doc\el"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\el\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 #SetOutPath "$INSTDIR\Resources\doc\eu"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\eu\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\eu\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\eu\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Customization.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\UserGuide.lyx"
 #SetOutPath "$INSTDIR\Resources\doc\ja"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\ja\Extended.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\linguistics.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\linguistics.lyx"
 #SetOutPath "$INSTDIR\Resources\layouts"
 #File "${PRODUCT_SOURCEDIR}\Resources\layouts\elsart.layout"
 #SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 #SetOutPath "$INSTDIR\Resources\templates"
 #File "${PRODUCT_SOURCEDIR}\Resources\templates\IEEEtran.lyx"

FunctionEnd

