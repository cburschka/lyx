Function DeleteFiles

 # list with deleted files for LyX 1.5.4
 Delete "$INSTDIR\bin\Microsoft.VC80.CRT.manifest"
 Delete "$INSTDIR\bin\msvcp80.dll"
 RMDir /r "$INSTDIR\Resources\doc\es\biblio"
 Delete "$INSTDIR\Resources\examples\mathed.lyx"
 Delete "$INSTDIR\Resources\examples\de\mathed.lyx"

FunctionEnd

