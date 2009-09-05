Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.5
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 #File "${PRODUCT_SOURCEDIR}\bin\QtCore4.dll"
 #File "${PRODUCT_SOURCEDIR}\bin\QtGui4.dll"
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\languages"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 #SetOutPath "$INSTDIR\Resources\bind"
 #File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\LFUNs.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\clipart"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\clipart\StandardToolbar.png"
 SetOutPath "$INSTDIR\Resources\doc\de"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Shortcuts.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\Braille.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\xypic.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\Braille.lyx"
 SetOutPath "$INSTDIR\Resources\examples\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\fr\Braille.lyx"
 #SetOutPath "$INSTDIR\Resources\layouts"
 #File "${PRODUCT_SOURCEDIR}\Resources\layouts\amsbook.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 #SetOutPath "$INSTDIR\Resources\scripts"
 #File "${PRODUCT_SOURCEDIR}\Resources\scripts\layout2layout.py"
 #SetOutPath "$INSTDIR\Resources\ui"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdcontext.inc"

FunctionEnd

