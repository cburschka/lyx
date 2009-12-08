Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.6
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\elyxer.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.css" # always here
 File "${PRODUCT_SOURCEDIR}\bin\setup.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\toc.css" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 #File "${PRODUCT_SOURCEDIR}\Resources\external_templates"
 #File "${PRODUCT_SOURCEDIR}\Resources\languages"
 #File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\bind"
 #File "${PRODUCT_SOURCEDIR}\Resources\bind\cua.bind"
 #File "${PRODUCT_SOURCEDIR}\Resources\bind\mac.bind"
 SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\LFUNs.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\ja"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\ja\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\sk"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\sk\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 #File /r "${PRODUCT_SOURCEDIR}\Resources\examples\ca"
 #File "${PRODUCT_SOURCEDIR}\Resources\examples\beamerlyxexample1.lyx"
 SetOutPath "$INSTDIR\Resources\images"
 #File "${PRODUCT_SOURCEDIR}\Resources\images\vc-repo-update.png"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\dinbrief.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\g-brief-de.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_6.py"
 SetOutPath "$INSTDIR\Resources\scripts"
 #File "${PRODUCT_SOURCEDIR}\Resources\scripts\ext_copy.py"
 #File "${PRODUCT_SOURCEDIR}\Resources\scripts\layout2layout.py"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\dinbrief.lyx"
 SetOutPath "$INSTDIR\Resources\ui"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdcontext.inc"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

