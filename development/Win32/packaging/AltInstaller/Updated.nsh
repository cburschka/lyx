Function UpdateModifiedFiles

 # list with modified files for LyX 1.6.2
 SetOutPath "$INSTDIR\bin" # always here
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\python26.dll"
 File "${PRODUCT_SOURCEDIR}\bin\python.exe"
 File /r "${PRODUCT_SOURCEDIR}\bin\DLLs"
 SetOutPath "$INSTDIR\bin\Lib"
 File "${PRODUCT_SOURCEDIR}\bin\Lib\optparse.py"
 SetOutPath "$INSTDIR\etc\Ghostscript"
 File /r "${PRODUCT_SOURCEDIR}\etc\Ghostscript"
 SetOutPath "$INSTDIR\Resources" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\external_templates"
 File "${PRODUCT_SOURCEDIR}\Resources\languages"
 File "${PRODUCT_SOURCEDIR}\Resources\symbols"
 SetOutPath "$INSTDIR\Resources\examples"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\biblioExample.bib"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\Braille.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\recipebook.lyx"
 SetOutPath "$INSTDIR\Resources\examples\de"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\Braille.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\examples\de\multicol.lyx"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\acmsiggraph.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\hanging.module"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\minimalistic.module"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\recipebook.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\revtex4.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\siamltex.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\stdinsets.inc"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\stdletter.inc"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_6.py"
 SetOutPath "$INSTDIR\Resources\man\man1"
 File "${PRODUCT_SOURCEDIR}\Resources\man\man1\tex2lyx.1"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\ACM-siggraph.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\letter.lyx"
 SetOutPath "$INSTDIR\Resources\ui"
 File "${PRODUCT_SOURCEDIR}\Resources\ui\stdcontext.inc"

FunctionEnd

