/* OS/2 REXX : The lib/configure.cmd; Don't delete this comment. */
call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs
env='OS2ENVIRONMENT'
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
    call SysMkDir 'clipart'
    call SysMkDir 'doc'
    call SysMkDir 'examples'
    call SysMkDir 'images'
    call SysMkDir 'kbd'
    call SysMkDir 'layouts'
    call SysMkDir 'reLyX'
    call SysMkDir 'scripts'
    call SysMkDir 'templates'
    call SysMkDir 'ui'
  end
  'cmd /c copy /v' srcdir'\layouts\*.layout layouts'
  if \(rc=0) then
    say 'Error! copying layout files.'
end
curdir=translate(curdir,'/','\')
srcdir=translate(srcdir,'/','\')
A_.0=0
call SEARCH_PROG 'converter latex dvi%latex|latex2e%"latex,disable=linuxdoc&docbook"'
call SEARCH_PROG 'converter latex pdf%pdflatex|pdflatex2e%"latex,disable=linuxdoc&docbook"'
call SEARCH_PROG 'converter latex html%tth -t -e2 -L$$BaseName < $$FName > $$OutName|latex2html -no_subdir -split 0 -show_section_numbers $$FName|hevea -s $$FName%"originaldir,needaux,disable=linuxdoc&docbook"'
call SEARCH_PROG 'converter literate latex%noweave -delay -index $$FName > $$OutName%""'
call SEARCH_PROG 'converter dvi ps%dvips -o $$OutName $$FName%""'
call SEARCH_PROG 'converter ps pdf%ps2pdf $$FName%""'
call SEARCH_PROG 'converter ps tiff%gs -sDEVICE=tiffg3 -sOutputFile=$$OutName -dNOPAUSE $$FName -c quit|gsos2 -sDEVICE=tiffg3 -sOutputFile=$$OutName -dNOPAUSE $$FName -c quit%""'
call SEARCH_PROG 'converter linuxdoc lyx%sgml2lyx $$FName%""'
call SEARCH_PROG 'converter linuxdoc latex%sgml2latex $$FName%""'
call SEARCH_PROG 'converter linuxdoc dvi%sgml2latex -o dvi $$FName%""'
call SEARCH_PROG 'converter linuxdoc html%sgml2html $$FName%""'
call SEARCH_PROG 'converter docbook dvi%sgmltools -b dvi $$FName|db2dvi $$FName%""'
call SEARCH_PROG 'converter docbook html%sgmltools -b html $$FName|db2html $$FName%""'
A_.0=A_.0+1;i=A_.0
A_.i=''
call SEARCH_PROG 'converter latex lyx%reLyX -f $$FName%"importer"'
call SEARCH_PROG 'converter literate lyx%noweb2lyx $$FName $$OutName%"importer"'
call SEARCH_PROG 'converter html latex%html2latex $$FName%"importer"'
call SEARCH_PROG 'converter word latex%wvCleanLatex $$FName $$OutName|word2x -f latex $$FName%"importer"'
A_.0=A_.0+1;i=A_.0
A_.i=''
call SEARCH_PROG 'converter gif eps%convert $$FName $$OutName%""'
call SEARCH_PROG 'converter png eps%convert $$FName $$OutName%""'
call SEARCH_PROG 'converter jpg eps%convert $$FName $$OutName%""'
call SEARCH_PROG 'converter gif png%convert $$FName $$OutName%""'
A_.0=A_.0+1;i=A_.0
A_.i=''
call SEARCH_PROG 'viewer dvi%xdvi'
call SEARCH_PROG 'viewer html%netscape'
call SEARCH_PROG 'viewer pdf%xpdf gv ghostview acroread'
call SEARCH_PROG 'viewer ps%gv -swap|ghostview -swap|gvpm -swap'
call SEARCH_PROG 'viewer eps%gv|ghostview|gvpm'
A_.0=A_.0+1;i=A_.0
A_.i=''
call SEARCH_PROG 'ps_command%gs|gsos2'
call SEARCH_PROG 'ascii_roff_command%groff -t -Tlatin1 $$FName'
call SEARCH_PROG 'chktex_command%chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38'
call SEARCH_PROG 'spell_command%ispell'
/* call SEARCH_PROG 'fax_command%fsend -p$$Phone -i$$FName -d$$Name -m$$Comment' */
call SEARCH_PROG 'print_spool_command%lp|lpr'
i=A_.0;A_.0=A_.0+1
if pos('lpr',A_.i)>0 then
  do
    i=i+1
    A_.i='print_spool_printerprefix "-P"'
  end
else
  do
    i=i+1
    A_.i='print_spool_printerprefix "-d "'
  end
/* call SysFileTree 'reLyX\reLyX.cmd', 'found', 'F' */
/* $ChkLaTeX
\nonstopmode\makeatletter
\ifx\undefined\documentclass\else
  \message{ThisIsLaTeX2e}
\fi
\@@end
 EOF */
call WRITE_FILE 'chklatex.ltx' '$ChkLaTeX'
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
/* $TextClass
# This file declares layouts and their associated definition files
# (include dir. relative to the place where this file is).
# It contains only default values, since chkconfig.ltx could not be run
# for some reason. Run ./configure if you need to update it after a
# configuration change.
article	article	article
report	report	report
book	book	book
linuxdoc	linuxdoc	linuxdoc
letter	letter	letter
 EOF */
  call WRITE_FILE 'textclass.lst' '$TextClass'
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
/* $Packages
# This file should contain the list of LaTeX packages that have been
# recognized by LyX. Unfortunately, since configure could not find
# your LaTeX2e program, the tests have not been run. Run configure
# if you need to update it after a configuration change.
 EOF */
  call WRITE_FILE 'packages.lst' '$Packages'
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
if rxfuncquery(SysAddRexxMacro) then
  rc=stream(lyxrc_defaults,'c','seek =0')
rc=SysFileDelete(X11ROOT'\XFree86\bin\fsend2.cmd')
/* $LyXRC_Default
# This file has been automatically generated by LyX' lib/configure
# script. It contains default settings that have been determined by
# examining your system. PLEASE DO NOT MODIFY ANYTHING HERE! If you
# want to customize LyX, make a copy of the file LYXDIR/lyxrc as
# ~/.lyx/lyxrc and edit this file instead. Any setting in lyxrc will
# override the values given here.
\Format latex tex     LaTeX           L
\Format dvi   dvi     DVI             D
\Format ps    ps      Postscript      t
\Format pdf   pdf     PDF             P
\Format html  html    HTML            H
\Format text  txt     ASCII           A
\Format word  doc     Word            W
\Format literate nw   NoWeb           N
\Format linuxdoc sgml LinuxDoc        x
\Format docbook  sgml DocBook         B
\Format program  ""   Program         ""
\Format fax      ""   Fax             ""

 EOF */
call WRITE_FILE lyxrc_defaults '$LyXRC_Default'
A_.0=A_.0+1;i=A_.0
A_.i='font_encoding "'||rc_fontenc||'"'
do i=1 to A_.0
  if A_.i\='' then A_.i='\'||A_.i
  call lineout lyxrc_defaults,A_.i
end
call lineout lyxrc_defaults
call lineout script
exit

SEARCH_PROG: procedure expose A_.
parse arg LABEL '%' COMMANDS '%' ROUTE
  i=A_.0+1
  A_.0=i
  A_.i=LABEL||' "none" '||ROUTE
  do while COMMANDS\=''
    parse value COMMANDS with COMMAND '|' COMMANDS
    parse upper value COMMAND with CMD  ARGV
    if (\(''=SysSearchPath('PATH',CMD||'.EXE'))|\(''=SysSearchPath('PATH',CMD||'.CMD'))) then
    do
      A_.i=LABEL||' "'||COMMAND||'" '||ROUTE
      COMMANDS=''
    end
  end
return

WRITE_FILE: procedure expose script
parse arg FILE TAG
call SysFileDelete FILE
C_=''
do while pos(TAG,C_)=0
  C_=linein(script)
end
C_=linein(script)
do while pos('EOF',C_)=0
  call lineout FILE,C_
  C_=linein(script)
end
call lineout FILE
/*call lineout script*/
return
