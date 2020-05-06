/*

filelist.nsh

Lists of files to include in the installer

*/

!define FILE '${COMMAND} "${DIRECTORY}'

!macro FileListLyXBin COMMAND DIRECTORY

  ${FILE}${BIN_LYX}"
  ${FILE}tex2lyx.exe"

!macroend

!macro FileListQtBin COMMAND DIRECTORY

  ${FILE}Qt5Concurrent.dll"
  ${FILE}Qt5Core.dll"
  ${FILE}Qt5Gui.dll"
  ${FILE}Qt5Network.dll"
  ${FILE}Qt5OpenGL.dll"
  ${FILE}Qt5PrintSupport.dll"
  ${FILE}Qt5Svg.dll"
  ${FILE}Qt5Widgets.dll"
  ${FILE}Qt5WinExtras.dll"

!macroend

!macro FileListQtImageformats COMMAND DIRECTORY

  ${FILE}qgif.dll"
  ${FILE}qico.dll"
  ${FILE}qjpeg.dll"
  ${FILE}qsvg.dll"
  ${FILE}qicns.dll"
  ${FILE}qtiff.dll"
  ${FILE}qwbmp.dll"
  ${FILE}qwebp.dll"

!macroend

!macro FileListQtIconengines COMMAND DIRECTORY

  ${FILE}qsvgicon.dll"

!macroend

!macro FileListQtPlatforms COMMAND DIRECTORY

  ${FILE}qminimal.dll"
  ${FILE}qwindows.dll"

!macroend

!macro FileListQtStyles COMMAND DIRECTORY

  ${FILE}qwindowsvistastyle.dll"

!macroend

!macro FileListMinGW COMMAND DIRECTORY

  # Files needed for MinGW
  ${FILE}iconv.dll"
  ${FILE}libbz2-1.dll"
  ${FILE}libfreetype-6.dll"
  ${FILE}libgcc_s_sjlj-1.dll"
  ${FILE}libglib-2.0-0.dll"
  ${FILE}libharfbuzz-0.dll"
  ${FILE}libintl-8.dll"
  ${FILE}libjpeg-62.dll"
  ${FILE}libpcre-1.dll"
  ${FILE}libpcre2-16-0.dll"
  ${FILE}libpng16-16.dll"
  ${FILE}libstdc++-6.dll"
  ${FILE}libtiff-5.dll"
  ${FILE}libwinpthread-1.dll"
  ${FILE}libwebp-7.dll"
  ${FILE}libwebpdemux-2.dll"
  ${FILE}zlib1.dll"

!macroend

!macro FileListMSVC COMMAND DIRECTORY

  # Files needed for MSVC 2019
  # Seem to be installed for dependencies
  ${FILE}concrt140.dll"
  ${FILE}msvcp140.dll"
  ${FILE}vcamp140.dll"
  ${FILE}vccorlib140.dll"
  ${FILE}vcomp140.dll"
  ${FILE}vcruntime140.dll"
  !if ${APP_VERSION_ACHITECHTURE} = 64
    ${File}vcruntime140_1.dll"
  !endif

!macroend

!macro FileListNetpbmBin COMMAND DIRECTORY

  ${FILE}libnetpbm10.dll"
  ${FILE}pnmcrop.exe"
  
!macroend

!macro FileListRsvg COMMAND DIRECTORY

  ${FILE}rsvg-convert.exe"
  
!macroend

!macro FileListUnoConv COMMAND DIRECTORY

  ${FILE}unoconv.py"
  
!macroend

!macro FileListPDFViewBin COMMAND DIRECTORY

  ${FILE}pdfview.exe"

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
