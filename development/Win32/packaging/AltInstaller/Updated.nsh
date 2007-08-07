Function UpdateModifiedFiles

 # list with modified files for LyX 1.6svn xx-xx-2007
 SetOutPath "$INSTDIR\bin"
 File "${PRODUCT_SOURCEDIR}\bin\lyx.exe" # always here
 File "${PRODUCT_SOURCEDIR}\bin\tex2lyx.exe" # always here
 File /r "${PRODUCT_SOURCEDIR}\Resources\locale" # always here

FunctionEnd

