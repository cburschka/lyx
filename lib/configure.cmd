/* OS/2 REXX : The lib/configure.cmd; Don't delete this comment. */
call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs
env='OS2ENVIRONMENT'
EMTEXDIR=value('EMTEXDIR',,env)
X11ROOT=value('X11ROOT',,env)
latex_script='chkconfig.ltx'
lyxrc_defaults='lyxrc.defaults'
lyx_check_config='yes'
rc_fontenc='default'

parse arg option '=' srcdir
select
   when option='-h'|option='-help'|option='--help' then do
  say 'Usage: configure [options]'
  say 'Options:'
  say '  --help                   show this help lines'
  say '  --without-latex-config   do not run LaTeX to determine configuration'
  exit
  end
   when option='--without-latex-config' then
  lyx_check_config='no'
   when option='--srcdir' then
  srcdir=translate(srcdir,'\','/')
   otherwise
  nop
end  /* select */
if srcdir='' then do
   parse source 'OS/2 COMMAND ' script
say 'script =' script
   srcdir=filespec('drive',script)strip(filespec('path',script),'T','\')
end

rc=SysFileTree(srcdir'\'latex_script,found,'F')
if found.0 = 0 then do
   say "configure: error: cannot find "srcdir'\'latex_script" script"
   exit 1
end
curdir=directory()
if \(curdir=srcdir) then do
  call SysFileTree 'layouts', 'found', 'D'
  if found.0 = 0 then do
    call SysMkDir 'bind'
    call SysMkDir 'doc'
    call SysMkDir 'kbd'
    call SysMkDir 'layouts'
    call SysMkDir 'templates'
  end
  'cmd /c copy /v' srcdir'\layouts\*.layout layouts'
  if \(rc=0) then
    say 'Error! copying layout files.'
end
curdir=translate(curdir,'/','\')
srcdir=translate(srcdir,'/','\')
call SysFileTree EMTEXDIR'\bin\latex.cmd', 'found', 'F'
if found.0 = 0 then LATEX='latex2e'
else LATEX='latex'
if \(''=SysSearchPath('PATH','GS.EXE')) then
  GS='gs'
else
if \(''=SysSearchPath('PATH','GSOS2.EXE')) then
  GS='gsos2'
else
  GS='none'
if \(''=SysSearchPath('PATH','GV.EXE')) then
  GHOSTVIEW='gv'
else
if \(''=SysSearchPath('PATH','GHOSTVIEW.EXE')) then
  GHOSTVIEW='ghostview'
else
  GHOSTVIEW='none'
if (''=SysSearchPath('PATH','SGML2LYX.CMD')) then
  SGMLTOOLS='none'
else
  SGMLTOOLS='sgml2lyx'
if (''=SysSearchPath('PATH','CHKTEX.EXE')) then
  CHKTEX='none'
else
  CHKTEX='chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38'
if \(''=SysSearchPath('PATH','ISPELL.EXE')) then
  SPELL='ispell'
else
  SPELL='none'
if (''=SysSearchPath('PATH','NOWEAVE.EXE')) then do
  LITERATE='none'
  LITERATE_EXT='none'
end
else do
  LITERATE='noweave -delay -index'
  LITERATE_EXT='.nw'
end
call SysFileTree 'reLyX\reLyX.cmd', 'found', 'F'
if found.0 = 0 then
  if (''=SysSearchPath('PATH','reLyX.cmd')) then
    RELYX='none'
  else
    RELYX='reLyX.cmd'
else
  RELYX='reLyX.cmd'
call SysFileDelete 'chklatex.ltx'
rc=lineout('chklatex.ltx','\nonstopmode\makeatletter')
rc=lineout('chklatex.ltx','\ifx\undefined\documentclass\else')
rc=lineout('chklatex.ltx','  \message{ThisIsLaTeX2e}')
rc=lineout('chklatex.ltx','\fi')
rc=lineout('chklatex.ltx','\@@end')
rc=lineout('chklatex.ltx')
'cmd /c 'LATEX' chklatex.ltx>nul'
call SysFileSearch 'NotLaTeX2e', 'chklatex.log', chklatex
if chklatex.0 > 0 then LATEX=''

rc=SysFileDelete('textclass.lst')
rc=SysFileDelete('packages.lst')
rc=SysFileDelete('chkconfig.sed')
if \(lyx_check_config='no') then
do
  rc=SysFileTree(srcdir'\layouts\*.layout',found,'FO')
  rc=SysFileDelete('chklayouts.tex')
  rc=SysFileDelete('chkconfig.vars')
  if found.0>0 then
    do num=1 to found.0
      parse value filespec('name',found.num) with DocClass '.layout'
      parse value DocClass with mainClass '_' subClass
      if ''=subClass then
        rc=lineout('chklayouts.tex','\TestDocClass{'DocClass'}')
      else	/* Not used, or must be empty? */
        rc=lineout('chklayouts.tex','\TestDocClass['mainClass'.cls]{'DocClass'}')
    end
  rc=lineout('chklayouts.tex')
  sgmltools_cmd=''
  chk_sgmltools='no'
  if SGMLTOOLS='sgml2lyx' then
   do
    chk_sgmltools='yes'
    sgmltools_cmd='\def\hassgmltools{}'
   end
  rc=SysFileDelete('wrap_chkconfig.ltx')
  rc=lineout('wrap_chkconfig.ltx','\newcommand\srcdir{'srcdir'}')
  rc=lineout('wrap_chkconfig.ltx',sgmltools_cmd)
  rc=lineout('wrap_chkconfig.ltx','\input{'srcdir'/chkconfig.ltx}')
  rc=lineout('wrap_chkconfig.ltx')
  'cmd /c 'LATEX' wrap_chkconfig.ltx'
  size=stream('chkconfig.sed','c','query size')
  rc=stream('chkconfig.sed','c','open')
  if rxfuncquery(SysAddRexxMacro) then
    rc=stream('chkconfig.sed','c','seek +'size)
  else
    rc=stream('chkconfig.sed','c','seek + 'size' write')
  rc=lineout('chkconfig.sed','s/@chk_sgmltools@/'chk_sgmltools'/g')
  rc=lineout('chkconfig.sed')
  call SysFileSearch 'chk_fontenc=', 'chkconfig.vars', fontenc
  if fontenc.0 > 0 then parse var fontenc.1 chk"='"rc_fontenc"'"
end

call SysFileTree 'textclass.lst', 'file', 'FO'
if file.0=0 then
do
   rc=lineout('textclass.lst','# This file declares layouts and their associated definition files')
   rc=lineout('textclass.lst','# (include dir. relative to the place where this file is).')
   rc=lineout('textclass.lst','# It contains only default values, since chkconfig.ltx could not be run')
   rc=lineout('textclass.lst','# for some reason. Run ./configure if you need to update it after a')
   rc=lineout('textclass.lst','# configuration change.')
   rc=lineout('textclass.lst','article	article	article')
   rc=lineout('textclass.lst','report	report	report')
   rc=lineout('textclass.lst','book	book	book')
   rc=lineout('textclass.lst','linuxdoc	linuxdoc	linuxdoc')
   rc=lineout('textclass.lst','letter	letter	letter')
   rc=lineout('textclass.lst')
   rc=lineout('chkconfig.sed','s/@.*@/???/g')
   rc=lineout('chkconfig.sed')
   rc_graphics='none'
end

call SysFileTree 'chkconfig.sed', 'file', 'FO'
if file.0=0 then
do
   rc=lineout('chkconfig.sed','s/@.*@/???/g')
   rc=lineout('chkconfig.sed')
end

call SysFileTree 'packages.lst', 'file', 'FO'
if file.0=0 then
do
   rc=lineout('packages.lst','# This file should contain the list of LaTeX packages that have been')
   rc=lineout('packages.lst','# recognized by LyX. Unfortunately, since configure could not find')
   rc=lineout('packages.lst','# your LaTeX2e program, the tests have not been run. Run configure')
   rc=lineout('packages.lst','# if you need to update it after a configuration change.')
   rc=lineout('packages.lst')
end

say 'creating doc/LaTeXConfig.lyx'
'sed -f chkconfig.sed 'srcdir'/doc/LaTeXConfig.lyx.in >doc\LaTeXConfig.lyx'
call SysFileDelete 'chkconfig.sed'
call SysFileDelete 'chkconfig.vars'
call SysFileDelete 'wrap_chkconfig.ltx'
call SysFileDelete 'wrap_chkconfig.log'
call SysFileDelete 'chklatex.ltx'
call SysFileDelete 'chklatex.log'
call SysFileDelete 'chklayouts.tex'
call SysFileDelete 'missfont.log'

say 'creating 'lyxrc_defaults
rc=SysFileDelete(lyxrc_defaults)
if rxfuncquery(SysAddRexxMacro) then
  rc=stream(lyxrc_defaults,'c','seek =0')
rc=SysFileDelete(X11ROOT'\XFree86\bin\fsend2.cmd')
rc=lineout(lyxrc_defaults,'\latex_command "'LATEX'"')
rc=lineout(lyxrc_defaults,'\relyx_command "'RELYX'"')
rc=lineout(lyxrc_defaults,'\literate_command "'LITERATE'"')
rc=lineout(lyxrc_defaults,'\literate_extension "'LITERATE_EXT'"')
rc=lineout(lyxrc_defaults,'\ps_command "'GS'"')
rc=lineout(lyxrc_defaults,'\view_ps_command "'GHOSTVIEW' -swap"')
rc=lineout(lyxrc_defaults,'\view_pspic_command "'GHOSTVIEW'"')
rc=lineout(lyxrc_defaults,'\ascii_roff_command "groff -t -ms -Tlatin1"')
rc=lineout(lyxrc_defaults,'\chktex_command "'CHKTEX'"')
rc=lineout(lyxrc_defaults,'\spell_command "'SPELL'"')
if \(''=SysSearchPath('PATH','fsend.exe')) then
do
  rc=lineout(X11ROOT'\XFree86\bin\fsend2.cmd','/*A wrapper for fsend.exe*/')
  rc=lineout(X11ROOT'\XFree86\bin\fsend2.cmd','parse arg "-p" phone "-i" fname "-d" name "-m" comment')
  rc=lineout(X11ROOT'\XFree86\bin\fsend2.cmd','"gs -sDEVICE=tiffg3 -sOutputFile=temp_tiff.TIF -dNOPAUSE "fname" -c quit"')
  rc=lineout(X11ROOT'\XFree86\bin\fsend2.cmd','"fsend -p"phone" -itemp_tiff.TIF -d"name" -m"comment')
  rc=lineout(X11ROOT'\XFree86\bin\fsend2.cmd','"rm -f temp_tiff.TIF"')
  rc=lineout(X11ROOT'\XFree86\bin\fsend2.cmd')
  rc=lineout(lyxrc_defaults,'\fax_command "fsend2 -p'"'$$Phone' -i'$$FName' -d'$$Name' -m'$$Comment'"'"')
end
if \(''=SysSearchPath('PATH','lp.exe')) then
do
  rc=lineout(lyxrc_defaults,'\print_spool_command "lp"')
  rc=lineout(lyxrc_defaults,'\print_spool_printerprefix "-d"')
end
else
if \(''=SysSearchPath('PATH','lpr.exe')) then
do
  rc=lineout(lyxrc_defaults,'\print_spool_command "lpr"')
  rc=lineout(lyxrc_defaults,'\print_spool_printerprefix "-P"')
end
else
do
  rc=lineout(lyxrc_defaults,'\print_spool_command ""')
  rc=lineout(lyxrc_defaults,'\print_spool_printerprefix ""')
end
rc=lineout(lyxrc_defaults,'\font_encoding "'rc_fontenc'"')
rc=lineout(lyxrc_defaults)
