/*

Lists of files to include in the installer

*/

!macro FileListLyXBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}lyx.exe"
  ${COMMAND} "${DIRECTORY}tex2lyx.exe"

!macroend

!macro FileListLyXIcons COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}lyx_32x32.ico"
  ${COMMAND} "${DIRECTORY}lyx_doc_32x32.ico"

!macroend

!macro FileListLyXScriptsCleanDVI COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}clean_dvi.py"

!macroend

!macro FileListQtBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}qt-mt3.dll"

!macroend

!macro FileListMSVCBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}msvcr80.dll"
  ${COMMAND} "${DIRECTORY}msvcp80.dll"
  
!macroend

!macro FileListMSVCManifest COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}\Microsoft.VC80.CRT.manifest"
  
!macroend

!macro FileListMinGWBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}mingwm10.dll"
  
!macroend

!macro FileListDllMSVCBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}iconv.dll"
  ${COMMAND} "${DIRECTORY}intl.dll"
  ${COMMAND} "${DIRECTORY}aspell.dll"
  ${COMMAND} "${DIRECTORY}Aiksaurus.dll"
  ${COMMAND} "${DIRECTORY}zlib1.dll"

!macroend

!macro FileListDllMinGWBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}libiconv-2.dll"
  ${COMMAND} "${DIRECTORY}libintl-3.dll"
  ${COMMAND} "${DIRECTORY}libAiksaurus-1-2-0.dll"
  ${COMMAND} "${DIRECTORY}libaspell-15.dll"
  ${COMMAND} "${DIRECTORY}zlib1.dll"

!macroend

!macro FileListNetpbmBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}libnetpbm10.dll"
  ${COMMAND} "${DIRECTORY}pnmcrop.exe"
  
!macroend

!macro FileListDTLBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}dt2dv.exe"
  ${COMMAND} "${DIRECTORY}dv2dt.exe"
  
!macroend

!macro FileListDvipostBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}pplatex.exe"
  
!macroend

!macro FileListWinScripts COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}clean_dvi.py"

!macroend

!macro FileListFonts COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}cmex10.ttf"
  ${COMMAND} "${DIRECTORY}cmmi10.ttf"
  ${COMMAND} "${DIRECTORY}cmr10.ttf"
  ${COMMAND} "${DIRECTORY}cmsy10.ttf"
  ${COMMAND} "${DIRECTORY}eufm10.ttf"
  ${COMMAND} "${DIRECTORY}msam10.ttf"
  ${COMMAND} "${DIRECTORY}msbm10.ttf"
  ${COMMAND} "${DIRECTORY}wasy10.ttf"
  
!macroend

!macro FileListPythonBin COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}python.exe"

!macroend

!macro FileListPythonDll COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}python25.dll"

!macroend

!macro FileListPythonLib COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}UserDict.py"
  ${COMMAND} "${DIRECTORY}codecs.py"
  ${COMMAND} "${DIRECTORY}copy_reg.py"
  ${COMMAND} "${DIRECTORY}fnmatch.py"
  ${COMMAND} "${DIRECTORY}getopt.py"
  ${COMMAND} "${DIRECTORY}glob.py"
  ${COMMAND} "${DIRECTORY}gzip.py"
  ${COMMAND} "${DIRECTORY}linecache.py"
  ${COMMAND} "${DIRECTORY}locale.py"
  ${COMMAND} "${DIRECTORY}ntpath.py"
  ${COMMAND} "${DIRECTORY}os.py"
  ${COMMAND} "${DIRECTORY}pipes.py"
  ${COMMAND} "${DIRECTORY}posixpath.py"
  ${COMMAND} "${DIRECTORY}random.py"
  ${COMMAND} "${DIRECTORY}re.py"
  ${COMMAND} "${DIRECTORY}shutil.py"
  ${COMMAND} "${DIRECTORY}site.py"
  ${COMMAND} "${DIRECTORY}sre.py"
  ${COMMAND} "${DIRECTORY}sre_compile.py"
  ${COMMAND} "${DIRECTORY}sre_constants.py"
  ${COMMAND} "${DIRECTORY}sre_parse.py"
  ${COMMAND} "${DIRECTORY}stat.py"
  ${COMMAND} "${DIRECTORY}string.py"
  ${COMMAND} "${DIRECTORY}struct.py"  
  ${COMMAND} "${DIRECTORY}tempfile.py"
  ${COMMAND} "${DIRECTORY}tempfile.py"
  ${COMMAND} "${DIRECTORY}types.py"
  ${COMMAND} "${DIRECTORY}warnings.py"

!macroend

!macro FileListPythonLibEncodings COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}__init__.py"
  ${COMMAND} "${DIRECTORY}aliases.py"
  ${COMMAND} "${DIRECTORY}cp1252.py"
  ${COMMAND} "${DIRECTORY}zlib_codec.py"
  
!macroend

!macro FileListAiksaurusData COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}words.dat"
  ${COMMAND} "${DIRECTORY}meanings.dat"
  
!macroend
