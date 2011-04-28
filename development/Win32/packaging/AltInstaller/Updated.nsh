Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.10
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\elyxer.py" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.css" # always here
 File "${PRODUCT_SOURCEDIR}\bin\math.css" # always here
 File "${PRODUCT_SOURCEDIR}\bin\setup.py" # always here
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 #SetOutPath "$INSTDIR\Resources\doc"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Customization.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Tutorial.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\sv"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\sv\Shortcuts.lyx"
 SetOutPath "$INSTDIR\Resources\examples\sv"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\sv\splash.lyx"
 #SetOutPath "$INSTDIR\Resources\layouts"
 #File "${PRODUCT_SOURCEDIR}\Resources\layouts\letter.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here

FunctionEnd

