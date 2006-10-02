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

  ${COMMAND} "${DIRECTORY}aliases.py"
  ${COMMAND} "${DIRECTORY}ascii.py"
  ${COMMAND} "${DIRECTORY}base64_codec.py"
  ${COMMAND} "${DIRECTORY}big5.py"
  ${COMMAND} "${DIRECTORY}big5hkscs.py"
  ${COMMAND} "${DIRECTORY}bz2_codec.py"
  ${COMMAND} "${DIRECTORY}charmap.py"
  ${COMMAND} "${DIRECTORY}cp037.py"
  ${COMMAND} "${DIRECTORY}cp1006.py"
  ${COMMAND} "${DIRECTORY}cp1026.py"
  ${COMMAND} "${DIRECTORY}cp1140.py"
  ${COMMAND} "${DIRECTORY}cp1250.py"
  ${COMMAND} "${DIRECTORY}cp1251.py"
  ${COMMAND} "${DIRECTORY}cp1252.py"
  ${COMMAND} "${DIRECTORY}cp1253.py"
  ${COMMAND} "${DIRECTORY}cp1254.py"
  ${COMMAND} "${DIRECTORY}cp1255.py"
  ${COMMAND} "${DIRECTORY}cp1256.py"
  ${COMMAND} "${DIRECTORY}cp1257.py"
  ${COMMAND} "${DIRECTORY}cp1258.py"
  ${COMMAND} "${DIRECTORY}cp424.py"
  ${COMMAND} "${DIRECTORY}cp437.py"
  ${COMMAND} "${DIRECTORY}cp500.py"
  ${COMMAND} "${DIRECTORY}cp737.py"
  ${COMMAND} "${DIRECTORY}cp775.py"
  ${COMMAND} "${DIRECTORY}cp850.py"
  ${COMMAND} "${DIRECTORY}cp852.py"
  ${COMMAND} "${DIRECTORY}cp855.py"
  ${COMMAND} "${DIRECTORY}cp856.py"
  ${COMMAND} "${DIRECTORY}cp857.py"
  ${COMMAND} "${DIRECTORY}cp860.py"
  ${COMMAND} "${DIRECTORY}cp861.py"
  ${COMMAND} "${DIRECTORY}cp862.py"
  ${COMMAND} "${DIRECTORY}cp863.py"
  ${COMMAND} "${DIRECTORY}cp864.py"
  ${COMMAND} "${DIRECTORY}cp865.py"
  ${COMMAND} "${DIRECTORY}cp866.py"
  ${COMMAND} "${DIRECTORY}cp869.py"
  ${COMMAND} "${DIRECTORY}cp874.py"
  ${COMMAND} "${DIRECTORY}cp875.py"
  ${COMMAND} "${DIRECTORY}cp932.py"
  ${COMMAND} "${DIRECTORY}cp949.py"
  ${COMMAND} "${DIRECTORY}cp950.py"
  ${COMMAND} "${DIRECTORY}euc_jisx0213.py"
  ${COMMAND} "${DIRECTORY}euc_jis_2004.py"
  ${COMMAND} "${DIRECTORY}euc_jp.py"
  ${COMMAND} "${DIRECTORY}euc_kr.py"
  ${COMMAND} "${DIRECTORY}gb18030.py"
  ${COMMAND} "${DIRECTORY}gb2312.py"
  ${COMMAND} "${DIRECTORY}gbk.py"
  ${COMMAND} "${DIRECTORY}hex_codec.py"
  ${COMMAND} "${DIRECTORY}hp_roman8.py"
  ${COMMAND} "${DIRECTORY}hz.py"
  ${COMMAND} "${DIRECTORY}idna.py"
  ${COMMAND} "${DIRECTORY}iso2022_jp.py"
  ${COMMAND} "${DIRECTORY}iso2022_jp_1.py"
  ${COMMAND} "${DIRECTORY}iso2022_jp_2.py"
  ${COMMAND} "${DIRECTORY}iso2022_jp_2004.py"
  ${COMMAND} "${DIRECTORY}iso2022_jp_3.py"
  ${COMMAND} "${DIRECTORY}iso2022_jp_ext.py"
  ${COMMAND} "${DIRECTORY}iso2022_kr.py"
  ${COMMAND} "${DIRECTORY}iso8859_1.py"
  ${COMMAND} "${DIRECTORY}iso8859_10.py"
  ${COMMAND} "${DIRECTORY}iso8859_11.py"
  ${COMMAND} "${DIRECTORY}iso8859_13.py"
  ${COMMAND} "${DIRECTORY}iso8859_14.py"
  ${COMMAND} "${DIRECTORY}iso8859_15.py"
  ${COMMAND} "${DIRECTORY}iso8859_16.py"
  ${COMMAND} "${DIRECTORY}iso8859_2.py"
  ${COMMAND} "${DIRECTORY}iso8859_3.py"
  ${COMMAND} "${DIRECTORY}iso8859_4.py"
  ${COMMAND} "${DIRECTORY}iso8859_5.py"
  ${COMMAND} "${DIRECTORY}iso8859_6.py"
  ${COMMAND} "${DIRECTORY}iso8859_7.py"
  ${COMMAND} "${DIRECTORY}iso8859_8.py"
  ${COMMAND} "${DIRECTORY}iso8859_9.py"
  ${COMMAND} "${DIRECTORY}johab.py"
  ${COMMAND} "${DIRECTORY}koi8_r.py"
  ${COMMAND} "${DIRECTORY}koi8_u.py"
  ${COMMAND} "${DIRECTORY}latin_1.py"
  ${COMMAND} "${DIRECTORY}mac_arabic.py"
  ${COMMAND} "${DIRECTORY}mac_centeuro.py"
  ${COMMAND} "${DIRECTORY}mac_croatian.py"
  ${COMMAND} "${DIRECTORY}mac_cyrillic.py"
  ${COMMAND} "${DIRECTORY}mac_farsi.py"
  ${COMMAND} "${DIRECTORY}mac_greek.py"
  ${COMMAND} "${DIRECTORY}mac_iceland.py"
  ${COMMAND} "${DIRECTORY}mac_latin2.py"
  ${COMMAND} "${DIRECTORY}mac_roman.py"
  ${COMMAND} "${DIRECTORY}mac_romanian.py"
  ${COMMAND} "${DIRECTORY}mac_turkish.py"
  ${COMMAND} "${DIRECTORY}mbcs.py"
  ${COMMAND} "${DIRECTORY}palmos.py"
  ${COMMAND} "${DIRECTORY}ptcp154.py"
  ${COMMAND} "${DIRECTORY}punycode.py"
  ${COMMAND} "${DIRECTORY}quopri_codec.py"
  ${COMMAND} "${DIRECTORY}raw_unicode_escape.py"
  ${COMMAND} "${DIRECTORY}rot_13.py"
  ${COMMAND} "${DIRECTORY}shift_jis.py"
  ${COMMAND} "${DIRECTORY}shift_jisx0213.py"
  ${COMMAND} "${DIRECTORY}shift_jis_2004.py"
  ${COMMAND} "${DIRECTORY}string_escape.py"
  ${COMMAND} "${DIRECTORY}tis_620.py"
  ${COMMAND} "${DIRECTORY}undefined.py"
  ${COMMAND} "${DIRECTORY}unicode_escape.py"
  ${COMMAND} "${DIRECTORY}unicode_internal.py"
  ${COMMAND} "${DIRECTORY}utf_16.py"
  ${COMMAND} "${DIRECTORY}utf_16_be.py"
  ${COMMAND} "${DIRECTORY}utf_16_le.py"
  ${COMMAND} "${DIRECTORY}utf_7.py"
  ${COMMAND} "${DIRECTORY}utf_8.py"
  ${COMMAND} "${DIRECTORY}utf_8_sig.py"
  ${COMMAND} "${DIRECTORY}uu_codec.py"
  ${COMMAND} "${DIRECTORY}zlib_codec.py"
  ${COMMAND} "${DIRECTORY}__init__.py"
  
!macroend

!macro FileListAiksaurusData COMMAND DIRECTORY

  ${COMMAND} "${DIRECTORY}words.dat"
  ${COMMAND} "${DIRECTORY}meanings.dat"
  
!macroend
