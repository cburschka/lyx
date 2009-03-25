Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.3
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 #File "${PRODUCT_SOURCEDIR}\bin\python26.dll"
 #File "${PRODUCT_SOURCEDIR}\bin\python.exe"
 #File /r "${PRODUCT_SOURCEDIR}\bin\DLLs"
 #SetOutPath "$INSTDIR\bin\Lib"
 #File "${PRODUCT_SOURCEDIR}\bin\Lib\optparse.py"
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\man\man1"
 #File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 #File "${PRODUCT_SOURCEDIR}\Resources\external_templates"
 #File "${PRODUCT_SOURCEDIR}\Resources\languages"
 #File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Extended.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Intro.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Intro.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\UserGuide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Intro.lyx"
 #SetOutPath "$INSTDIR\Resources\examples"
 SetOutPath "$INSTDIR\Resources\images"
 File "${PRODUCT_SOURCEDIR}\Resources\images\bookmark-goto_0.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_m-align-center.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_m-align-left.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_m-align-right.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_m-valign-bottom.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_m-valign-middle.png"
 File "${PRODUCT_SOURCEDIR}\Resources\images\tabular-feature_m-valign-top.png"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\stdinsets.inc"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_6.py"
 #SetOutPath "$INSTDIR\Resources\templates"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdcontext.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

