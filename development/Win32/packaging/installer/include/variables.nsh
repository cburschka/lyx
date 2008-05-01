/*

variables.nsh

Variables that are shared between multiple files

*/

Var PathLaTeX
Var PathLaTeXLocal
Var PathImageMagick
Var PathGhostscript

Var SetupLaTeX
Var SizeLaTeX

!ifndef BUNDLE_IMAGEMAGICK
  Var SetupImageMagick
  Var SizeImageMagick
!endif

!ifndef BUNDLE_GHOSTSCRIPT
  Var SetupGhostscript
  Var SizeGhostscript
!endif

Var LangName
Var LangISOCode

Var PrinterConf
