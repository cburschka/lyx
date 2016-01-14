/*

filelist.nsh

Lists of files to include in the installer

*/

!define FILE '${COMMAND} "${DIRECTORY}'

!macro FileListLyXBin COMMAND DIRECTORY

  ${FILE}lyx.exe"
  ${FILE}tex2lyx.exe"

!macroend

!macro FileListQtBin COMMAND DIRECTORY

  ${FILE}Qt5Core.dll"
  ${FILE}Qt5Gui.dll"
  ${FILE}Qt5Svg.dll"
  ${FILE}Qt5Widgets.dll"
  ${FILE}Qt5WinExtras.dll"
  ${FILE}Qt5Xml.dll"

!macroend

!macro FileListQtImageformats COMMAND DIRECTORY

  ${FILE}qdds.dll"
  ${FILE}qgif.dll"
  ${FILE}qicns.dll"
  ${FILE}qico.dll"
  ${FILE}qjp2.dll"
  ${FILE}qjpeg.dll"
  ${FILE}qmng.dll"
  ${FILE}qsvg.dll"
  ${FILE}qtga.dll"
  ${FILE}qtiff.dll"
  ${FILE}qwbmp.dll"
  ${FILE}qwebp.dll"

!macroend

!macro FileListQtIconengines COMMAND DIRECTORY

  ${FILE}qsvgicon.dll"

!macroend

!macro FileListMSVC COMMAND DIRECTORY

  # Manifest not needed for 2010
  ${FILE}msvcr100.dll"
  ${FILE}msvcp100.dll"
  
!macroend

!macro FileListDll COMMAND DIRECTORY

  ${FILE}iconv.dll"
  ${FILE}intl.dll"
  ${FILE}libhunspell.dll"
  ${FILE}zlibwapi.dll"

!macroend

!macro FileListNetpbmBin COMMAND DIRECTORY

  ${FILE}libnetpbm10.dll"
  ${FILE}pnmcrop.exe"
  
!macroend

!macro FileListDTLBin COMMAND DIRECTORY

  ${FILE}dt2dv.exe"
  ${FILE}dv2dt.exe"
  
!macroend

!macro FileListRsvg COMMAND DIRECTORY

  ${FILE}rsvg-convert.exe"
  
!macroend

!macro FileListDvipostBin COMMAND DIRECTORY

  ${FILE}pplatex.exe"
  
!macroend

!macro FileListUnoConv COMMAND DIRECTORY

  ${FILE}unoconv.py"
  
!macroend

!macro FileListeLyXer COMMAND DIRECTORY

  ${FILE}elyxer.py"
  ${FILE}lyx.css"
  ${FILE}math.css"
  ${FILE}math2html.py"
  ${FILE}toc.css"
  
!macroend

!macro FileListPDFToolsBin COMMAND DIRECTORY

  ${FILE}pdfopen.exe"
  ${FILE}pdfclose.exe"
  
!macroend

!macro FileListPDFViewBin COMMAND DIRECTORY

  ${FILE}pdfview.exe"
  ${FILE}System.dll"
  ${FILE}Console.dll"

!macroend

!macro FileListFonts COMMAND DIRECTORY
# FIXME: simply copy the whole directory!

  ${FILE}cmex10.ttf"
  ${FILE}cmmi10.ttf"
  ${FILE}cmr10.ttf"
  ${FILE}cmsy10.ttf"
  ${FILE}esint10.ttf"
  ${FILE}eufm10.ttf"
  ${FILE}msam10.ttf"
  ${FILE}msbm10.ttf"
  ${FILE}rsfs10.ttf"
  ${FILE}wasy10.ttf"
  
!macroend
