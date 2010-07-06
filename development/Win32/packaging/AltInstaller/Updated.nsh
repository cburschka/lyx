Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.7
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\elyxer.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.css" # always here
 File "${PRODUCT_SOURCEDIR}\bin\setup.py" # always here
 #File "${PRODUCT_SOURCEDIR}\bin\toc.css"
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 #File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 File "${PRODUCT_SOURCEDIR}\Resources\languages"
 File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 #SetOutPath "$INSTDIR\Resources\bind"
 #File "${PRODUCT_SOURCEDIR}\Resources\bind\aqua.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 #SetOutPath "$INSTDIR\Resources\doc\clipart"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\clipart\Abstract.pdf"
 #SetOutPath "$INSTDIR\Resources\doc\de"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 SetOutPath "$INSTDIR\Resources\doc\el"
 File "${PRODUCT_SOURCEDIR}\Resources\el\Intro.lyx"
 #SetOutPath "$INSTDIR\Resources\doc\es"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Customization.lyx"
 SetOutPath "$INSTDIR\Resources\doc\eu"
 File "${PRODUCT_SOURCEDIR}\Resources\eu\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\eu\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\eu\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\ja"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\ja\Extended.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\tufte-book.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\tufte-handout.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\tufte-handout.lyx"
 SetOutPath "$INSTDIR\Resources\examples\el"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\el\splash.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\elsart.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\IEEEtran.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\tufte-book.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\tufte-handout.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 SetOutPath "$INSTDIR\Resources\man\man1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\lyx.1"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\IEEEtran.lyx"
 #SetOutPath "$INSTDIR\Resources\ui"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

