Function UpdateModifiedFiles

 # list with modified files for LyX 1.5.6
 SetOutPath "$INSTDIR\bin" # always here
 #File "${PRODUCT_SOURCEDIR}\bin\Console.dll"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 #File "${PRODUCT_SOURCEDIR}\bin\pdfview.exe"
 #File "${PRODUCT_SOURCEDIR}\bin\System.dll"
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here
 SetOutPath "$INSTDIR\Resources"
 #File "${PRODUCT_SOURCEDIR}\Resources\chkconfig.ltx"
 File "${PRODUCT_SOURCEDIR}\Resources\configure.py"
 File "${PRODUCT_SOURCEDIR}\Resources\CREDITS"
 File "${PRODUCT_SOURCEDIR}\Resources\unicodesymbols"
 SetOutPath "$INSTDIR\Resources\doc"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\LaTeXConfig.lyx.in"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\de"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Math.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\de\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\es"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Extended.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Intro.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Math.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Tutorial.lyx"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\es\Userguide.lyx"
 SetOutPath "$INSTDIR\Resources\doc\fr"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\EmbeddedObjects.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\doc\fr\Extended.lyx"
 SetOutPath "$INSTDIR\Resources\doc\it"
 #File "${PRODUCT_SOURCEDIR}\Resources\doc\it\Intro.lyx"
 #File /r "${PRODUCT_SOURCEDIR}\Resources\doc\it\clipart"
 SetOutPath "$INSTDIR\Resources\layouts"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\aa.layout"
 File "${PRODUCT_SOURCEDIR}\Resources\layouts\jss.layout"
 SetOutPath "$INSTDIR\Resources\lyx2lyx" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\LyX.py"
 File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx2lyx_version.py" # always here
 #File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\lyx_1_5.py"
 #File "${PRODUCT_SOURCEDIR}\Resources\lyx2lyx\parser_tools.py"
 SetOutPath "$INSTDIR\Resources\scripts"
 File "${PRODUCT_SOURCEDIR}\Resources\scripts\csv2lyx.py"
 SetOutPath "$INSTDIR\Resources\templates"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\aa.lyx"
 File "${PRODUCT_SOURCEDIR}\Resources\templates\JSS-article.lyx"
 SetOutPath "$INSTDIR\Resources\man\man1"
 #File "${PRODUCT_SOURCEDIR}\Resources\man\man1\lyx.1"
 SetOutPath "$INSTDIR\Resources\ui"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdmenus.inc"
 #File "${PRODUCT_SOURCEDIR}\Resources\ui\stdtoolbars.inc"

FunctionEnd

