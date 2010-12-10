Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.9
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\elyxer.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.css" # always here
 File "${PRODUCT_SOURCEDIR}\bin\setup.py" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 #File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 #File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 #File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 #File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 #SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\modernCV.lyx"
 SetOutPath "$INSTDIR\Resources\examples\sr"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\sr\splash.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\lettre.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 #SetOutPath "$INSTDIR\Resources\templates"
 #File "${PRODUCT_SOURCEDIR}\Resources\templates\agutex.lyx"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\classic.ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdcontext.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

