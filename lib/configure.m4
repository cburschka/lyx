#! /bin/sh
dnl Use 'make' to create configure from this m4 script.
# This script is a hand-made configure script. It contains a lot of
# code stolen from GNU autoconf. I removed all the code that was not
# useful for configuring a LyX installation.


dnl ######### Begin M4 macros #########################################
dnl This is a template for my stripped-down configure script.
dnl First, a few convenient macros.
changequote([,])dnl
dnl
dnl
dnl MSG_CHECKING(FEATURE-DESCRIPTION,PREFIX)
dnl
define(MSG_CHECKING,
[echo $ac_n "$2checking $1""... $ac_c"])dnl
dnl
dnl
dnl MSG_RESULT(RESULT-DESCRIPTION)
dnl
define(MSG_RESULT,
[echo "$ac_t""$1"])dnl
dnl
dnl
dnl SEARCH_PROG(FEATURE-DESCRIPTION,VARIABLE-NAME,PROGRAMS-LIST,
dnl             ACTION-IF-FOUND,ACTION-IF-NOT-FOUND)
dnl
define(SEARCH_PROG,[dnl
changequote([,])dnl
MSG_CHECKING($1)
MSG_RESULT(($3))
$2=
for ac_prog in $3
do
# Extract the first word of "$ac_prog", so it can be a program name with args.
set dummy $ac_prog ; ac_word=$[2]
if test -n "$ac_word"; then
  MSG_CHECKING([for \"$ac_word\"],[+])
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -x [$ac_dir/$ac_word]; then
      $2="$ac_prog"
      break
    fi
  done
  IFS="$ac_save_ifs"

  if test -n "[$]$2"; then
    ac_result=yes
    ifelse($4,,,[$4])
  else
    ac_result=no
  fi
  MSG_RESULT($ac_result)
  test -n "[$]$2" && break
fi
done

if test -z "[$]$2" ; then
  $2=none
ifelse($5,,,[  $5
])dnl
fi
changequote(,)dnl
])dnl
dnl
dnl
dnl PROVIDE_DEFAULT_FILE(FILE, DEFAULT-VALUE)
dnl
define(PROVIDE_DEFAULT_FILE,[dnl
# if $1 does not exist (because LaTeX did not run), 
# then provide a standard version.
if test ! -f $1 ; then
  cat >$1 <<EOF
$2
EOF
fi])
changequote(,)dnl
dnl ######### End M4 macros #############################################


####some configuration variables
lyx_check_config=yes
lyx_keep_temps=no
srcdir=
lyx_suffix=

#### Parse the command line
for ac_option do
  case "$ac_option" in
    -help | --help | -h)
      cat << EOF
Usage: configure [options] 
Options: 
  --help                   show this help lines
  --keep-temps             keep temporary files (for debug. purposes)
  --without-latex-config   do not run LaTeX to determine configuration
  --with-lyx-suffix=suffix suffix of binary installed files
EOF
      exit 0;;
    --without-latex-config)
      lyx_check_config=no ;;
    --keep-temps)
      lyx_keep_temps=yes ;;
    --with-lyx-suffix*)
      lyx_suffix=`echo "$ac_option" | sed 's,--with-lyx-suffix=,,;s,^,-,'`
  esac 
done


#### Checking for some echo oddities
if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  # Stardent Vistra SVR4 grep lacks -e, says ghazi@caip.rutgers.edu.
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='	'
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi


#### I do not really know why this is useful, but we might as well keep it.
# NLS nuisances.
# Only set these to C if already set.  These must not be set unconditionally
# because not all systems understand e.g. LANG=C (notably SCO).
# Fixing LC_MESSAGES prevents Solaris sh from translating var values in `set'!
# Non-C LC_CTYPE values break the ctype check.
if test "${LANG+set}"   = set; then LANG=C;   export LANG;   fi
if test "${LC_ALL+set}" = set; then LC_ALL=C; export LC_ALL; fi
if test "${LC_MESSAGES+set}" = set; then LC_MESSAGES=C; export LC_MESSAGES; fi
if test "${LC_CTYPE+set}"    = set; then LC_CTYPE=C;    export LC_CTYPE;    fi


#### Guess the directory in which configure is located.
changequote([,])dnl
ac_prog=[$]0
changequote(,)dnl
srcdir=`echo $ac_prog|sed 's%/[^/][^/]*$%%'` 
srcdir=`echo "${srcdir}" | sed 's%\([^/]\)/*$%\1%'`
test "x$srcdir" = "x$ac_prog" && srcdir=.
if test ! -r ${srcdir}/chkconfig.ltx ; then
  echo "configure: error: cannot find chkconfig.ltx script"
  exit 1
fi

#### Adjust PATH for Win32 (Cygwin)
if test "x$OSTYPE" = xcygwin; then
  echo "configure: cygwin detected; path correction"
  srcdir=`cygpath -w "${srcdir}" | tr '\\\\' /`
  echo "srcdir=${srcdir}"
fi

#### Create the build directories if necessary
for dir in bind clipart doc examples help images kbd layouts reLyX \
    scripts templates ui ; do
  test ! -d $dir && mkdir $dir
done


#### Searching some useful programs
define(CHECKLATEX2E,[
## Check whether this is really LaTeX2e
rm -f chklatex.ltx
cat >chklatex.ltx <<EOF
\\nonstopmode\\makeatletter
\\ifx\\undefined\\documentclass\\else
  \\message{ThisIsLaTeX2e}
\\fi
\\@@end
EOF
if eval ${LATEX} chklatex.ltx </dev/null 2>/dev/null \
                       | grep 'ThisIsLaTeX2e' >/dev/null; then
  :
else
  LATEX=
  ac_result="not useable"
fi
rm -f chklatex.ltx chklatex.log])dnl
dnl
# Search LaTeX2e
SEARCH_PROG([for a LaTeX2e program],LATEX,latex latex2e,CHECKLATEX2E,dnl
  [lyx_check_config=no])
latex_to_dvi=$LATEX
test -z "$latex_to_dvi" && latex_to_dvi="none"

# Search for pdflatex
if test ${lyx_check_config} = no ; then
  latex_to_pdf=none
else
  SEARCH_PROG([for the pdflatex program],latex_to_pdf,pdflatex)
fi

test $latex_to_dvi != "none" && latex_to_dvi="$latex_to_dvi \$\$i"
test $latex_to_pdf != "none" && latex_to_pdf="$latex_to_pdf \$\$i"

# Search for an installed reLyX or a ready-to-install one
save_PATH=${PATH}
PATH=${PATH}:./reLyX/
SEARCH_PROG([for a LaTeX -> LyX converter],tex_to_lyx_command,reLyX)
PATH=${save_PATH}
test $tex_to_lyx_command = "reLyX" && tex_to_lyx_command="reLyX -f \$\$i"
tex_to_lyx_command=`echo $tex_to_lyx_command | sed "s,reLyX,reLyX$lyx_suffix,"`

SEARCH_PROG([for a Noweb -> LyX converter],literate_to_lyx_command,noweb2lyx)
test $literate_to_lyx_command = "noweb2lyx" && literate_to_lyx_command="noweb2lyx \$\$i \$\$o"
literate_to_lyx_command=`echo $literate_to_lyx_command | sed "s,noweb2lyx,noweb2lyx$lyx_suffix,"`

# Search something to process a literate document
SEARCH_PROG([for a Noweb -> LaTeX converter],literate_to_tex_command,noweave)
test $literate_to_tex_command = "noweave" && literate_to_tex_command="noweave -delay -index \$\$i > \$\$o"

SEARCH_PROG([for a HTML -> Latex converter],html_to_latex_command,html2latex)
test $html_to_latex_command = "html2latex" && html_to_latex_command="html2latex \$\$i"

SEARCH_PROG([for a MSWord -> Latex converter],word_to_latex_command,wvCleanLatex word2x)
test "$word_to_latex_command" = "wvCleanLatex" && word_to_latex_command="wvCleanLatex \$\$i \$\$o"
test "$word_to_latex_command" = "word2x" && word_to_latex_command="word2x -f latex \$\$i"

SEARCH_PROG([for Image converter],image_command,convert)
test $image_command = "convert" && image_command="convert \$\$i \$\$o"

# Search for a Postscript interpreter
SEARCH_PROG([for a Postscript interpreter],GS, gs)

# Search something to preview postscript
SEARCH_PROG([for a Postscript previewer],GHOSTVIEW,gv ghostview)

# Search for a program to preview pdf
SEARCH_PROG([for a PDF preview],PDF_VIEWER,acroread gv ghostview xpdf)

# Search something to preview dvi
SEARCH_PROG([for a DVI previewer],DVI_VIEWER, xdvi windvi yap)

# Search something to preview html
SEARCH_PROG([for a HTML previewer],HTML_VIEWER, netscape)

# Search for a program to convert ps to pdf
SEARCH_PROG([for a PS to PDF converter],ps_to_pdf_command,ps2pdf)
test $ps_to_pdf_command = "ps2pdf" && ps_to_pdf_command="ps2pdf \$\$i"

# Search for a program to convert dvi to ps
SEARCH_PROG([for a DVI to PS converter],dvi_to_ps_command,dvips)
test $dvi_to_ps_command = "dvips" && dvi_to_ps_command="dvips -o \$\$o \$\$i"

# Search for a program to convert dvi to pdf
SEARCH_PROG([for a DVI to PDF converter],dvi_to_pdf_command,dvipdfm)
test $dvi_to_pdf_command = "dvipdfm" && dvi_to_pdf_command="dvipdfm \$\$i"

# Search a *roff program (used to translate tables in ASCII export)
SEARCH_PROG([for a *roff formatter],ROFF,groff nroff)
ascii_roff_command=$ROFF
test $ROFF = "groff" && ascii_roff_command="groff -t -Tlatin1 \$\$FName"
test $ROFF = "nroff" && ascii_roff_command="tbl \$\$FName | nroff"

# Search the ChkTeX program
SEARCH_PROG([for ChkTeX],CHKTEX,chktex)
chktex_command=$CHKTEX
test $CHKTEX = "chktex" && chktex_command="$CHKTEX -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38"

# Search for a spellchecker
SEARCH_PROG([for a spell-checker], SPELL,ispell)

dnl # Search a Fax handling program
dnl SEARCH_PROG([for a fax driver], FAX, sendfax faxsend fax)
dnl if test $FAX = sendfax ; then
dnl   fax_command="sendfax -n -h '\$\$Host' -c '\$\$Comment' -x '\$\$Enterprise' -d '\$\$Name'@'\$\$Phone' '\$\$FName'"
dnl elif test $FAX = faxsend ; then
dnl   fax_command="faxsend '\$\$Phone' '\$\$FName'"
dnl elif test $FAX = fax ; then
dnl   fax_command="fax send '\$\$Phone' '\$\$FName'"
dnl else
dnl   fax_command="none"
dnl fi

# Search a GUI Fax program
SEARCH_PROG([for a fax program], fax_command, ksendfax)
test $fax_command = "ksendfax" && fax_command="ksendfax \$\$i"

# Search for LinuxDoc support
SEARCH_PROG([for SGML-tools 1.x (LinuxDoc)], LINUXDOC, sgml2lyx)
chk_linuxdoc=no
if test $LINUXDOC != none; then
  chk_linuxdoc=yes
  linuxdoc_cmd="\\def\\haslinuxdoc{yes}"
fi

case $LINUXDOC in
  sgml2lyx)
    linuxdoc_to_latex_command="sgml2latex \$\$i"
    linuxdoc_to_dvi_command="sgml2latex -o dvi \$\$i"
    linuxdoc_to_html_command="sgml2html \$\$i"
    linuxdoc_to_lyx_command="sgml2lyx \$\$i";;
  none)
    linuxdoc_to_latex_command="none"
    linuxdoc_to_dvi_command="none"
    linuxdoc_to_html_command="none"
    linuxdoc_to_lyx_command="none";;
esac

# Search for DocBook support
SEARCH_PROG([for SGML-tools 2.x (DocBook) or db2x scripts], DOCBOOK, sgmltools db2dvi)
chk_docbook=no
if test $DOCBOOK != none; then
  chk_docbook=yes
  docbook_cmd="\\def\\hasdocbook{yes}"
fi

case $DOCBOOK in
  sgmltools)
    docbook_to_dvi_command="sgmltools -b dvi \$\$i"
    docbook_to_html_command="sgmltools -b html \$\$i";;
  db2dvi)
    docbook_to_dvi_command="db2dvi \$\$i"
    docbook_to_html_command="db2html \$\$i";;
  none)
    docbook_to_dvi_command="none"
    docbook_to_html_command="none";;
esac

# Search for a spool command
SEARCH_PROG([for a spool command], LPR, lp lpr)
case $LPR in
  lp) print_spool_command=lp
      print_spool_printerprefix="-d ";;
 lpr) print_spool_command=lpr
      print_spool_printerprefix="-P";;
   *) :;; # leave to empty values
esac

SEARCH_PROG([for a LaTeX -> HTML converter], TOHTML, tth latex2html hevea)
latex_to_html_command=$TOHTML
case $TOHTML in
	tth) latex_to_html_command="tth -t -e2 -L\$\$b < \$\$i > \$\$o";;
 latex2html) latex_to_html_command="latex2html -no_subdir -split 0 -show_section_numbers \$\$i";;
      hevea) latex_to_html_command="hevea -s \$\$i";;
esac

#### Search for image conversion ####
SEARCH_PROG([for an Image -> EPS converter], TOEPS, convert pnmtops)
case $TOEPS in
	convert) gif_to_eps="convert GIF:\$\$i EPS:\$\$o" png_to_eps="convert PNG:\$\$i EPS:\$\$o" jpg_to_eps="convert JPG:\$\$i EPS:\$\$o";;
	pnmtops) gif_to_eps="giftopnm \$\$i | pnmtops > \$\$o" png_to_eps="pngtopnm \$\$i | pnmtops >\$\$o" jpg_to_eps="jpegtopnm \$\$i | pnmtops >\$\$o";;
esac

SEARCH_PROG([for a Image -> PNG converter], TOPNG, convert pnmtopng)
case $TOPNG in
	convert) gif_to_png="convert GIF:\$\$i PNG:\$\$o" eps_to_png="convert EPS:\$\$i PNG:\$\$o" jpg_to_png="convert JPG:\$\$i PNG:\$\$o";;
	pnmtopng) gif_to_png="giftopnm \$\$i | pnmtopng >\$\$o" eps_to_png="pstopnm \$\$i| pnmtopng >\$\$o" jpg_to_png="jpegtopnm \$\$i | pnmtopng >\$\$o";;
esac

SEARCH_PROG([for a Image -> XPM converter], TOXPM, convert)
if test "$TOXPM" = "convert"; then
	gif_to_xpm="convert GIF:\$\$i XPM:\$\$o"
	eps_to_xpm="convert EPS:\$\$i XPM:\$\$o" 
	jpg_to_xpm="convert JPG:\$\$i XPM:\$\$o"
	png_to_xpm="convert PNG:\$\$i XPM:\$\$o"
fi

SEARCH_PROG([For an EPS -> PDF converter], EPSTOPDF, epstopdf)
case $EPSTOPDF in
	epstopdf) eps_to_pdf="epstopdf --outfile=\$\$o \$\$i";;
esac

#### Explore the LaTeX configuration
MSG_CHECKING(LaTeX configuration)
# First, remove the files that we want to re-create
rm -f textclass.lst packages.lst chkconfig.sed
if test ${lyx_check_config} = no ; then
  MSG_RESULT(default values)
else
  MSG_RESULT(auto)
  rm -f wrap_chkconfig.ltx chkconfig.vars chkconfig.classes chklayouts.tex
  cat >wrap_chkconfig.ltx <<EOF
\\newcommand\\srcdir{${srcdir}}
${linuxdoc_cmd}
${docbook_cmd}
\\input{${srcdir}/chkconfig.ltx}
EOF
  ## Construct the list of classes to test for.
  # build the list of available layout files and convert it to commands 
  # for chkconfig.ltx 
  for file in ./layouts/*.layout ${srcdir}/layouts/*.layout ; do 
    case $file in
      */\*.layout) ;;
      *) test -r "$file" && echo $file ;;
    esac
  done | sed -e 's%^.*layouts/\(.*\)\.layout$%\\TestDocClass{\1}%'\
             > chklayouts.tex
changequote([,])dnl
  [eval] ${LATEX} wrap_chkconfig.ltx 2>/dev/null | grep '^\+'
  [eval] `cat chkconfig.vars | sed 's/-/_/g'`
changequote(,)dnl
fi

# Do we have all the files we need? Useful if latex did not run
changequote([,])dnl
echo creating textclass.lst 
PROVIDE_DEFAULT_FILE(textclass.lst,dnl
[# This file declares layouts and their associated definition files
# (include dir. relative to the place where this file is).
# It contains only default values, since chkconfig.ltx could not be run 
# for some reason. Run ./configure if you need to update it after a
# configuration change.  
article	article	article
report	report	report
book	book	book
linuxdoc	linuxdoc	linuxdoc
letter	letter	letter])

PROVIDE_DEFAULT_FILE(chkconfig.sed,[s/@.*@/???/g])

echo creating packages.lst
PROVIDE_DEFAULT_FILE(packages.lst,dnl
[# This file should contain the list of LaTeX packages that have been
# recognized by LyX. Unfortunately, since configure could not find
# your LaTeX2e program, the tests have not been run. Run ./configure
# if you need to update it after a configuration change.
])
changequote(,)dnl

echo creating doc/LaTeXConfig.lyx
echo "s/@chk_linuxdoc@/$chk_linuxdoc/g" >> chkconfig.sed
echo "s/@chk_docbook@/$chk_docbook/g" >> chkconfig.sed
sed -f chkconfig.sed ${srcdir}/doc/LaTeXConfig.lyx.in >doc/LaTeXConfig.lyx

echo creating lyxrc.defaults
rm -f lyxrc.defaults
cat >lyxrc.defaults <<EOF
# This file has been automatically generated by LyX' lib/configure
# script. It contains default settings that have been determined by
# examining your system. PLEASE DO NOT MODIFY ANYTHING HERE! If you
# want to customize LyX, make a copy of the file LYXDIR/lyxrc as
# ~/.lyx/lyxrc and edit this file instead. Any setting in lyxrc will
# override the values given here.
\\Format text	  txt	ASCII		A
\\Format textparagraph txt ASCII(paragraphs)	""
\\Format docbook  sgml	DocBook		B
\\Format dvi	  dvi	DVI		D
\\Format eps	  eps	EPS		""
\\Format epsi     epsi  EPSI    ""
\\Format fax	  ""	Fax		""
\\Format gif      gif	GIF		""
\\Format html	  html	HTML		H
\\Format jpg      jpg	JPEG		""
\\Format latex	  tex	LaTeX		L
\\Format linuxdoc sgml	LinuxDoc	x
\\Format lyx      lyx	LyX		""
\\Format literate nw	NoWeb		N
\\Format pdf	  pdf	PDF		P
\\Format pdf2	  pdf  "PDF (pdflatex)"	F
\\Format pdf3	  pdf  "PDF (dvipdfm)"	m
\\Format png	  png	PNG		""
\\Format ps	  ps	Postscript	t
\\Format program  ""	Program		""
\\Format xpm      xpm   XPM             ""
\\Format word	  doc	Word		W

\\converter latex dvi "$latex_to_dvi" "latex"
\\converter latex pdf2 "$latex_to_pdf" "latex"
\\converter latex html "$latex_to_html_command" "originaldir,needaux"
\\converter literate latex "$literate_to_tex_command" ""
\\converter dvi pdf3 "$dvi_to_pdf_command" ""
\\converter dvi ps "$dvi_to_ps_command" ""
\\converter ps pdf "$ps_to_pdf_command" ""
\\converter ps fax "$fax_command" ""
\\converter linuxdoc lyx "$linuxdoc_to_lyx_command" ""
\\converter linuxdoc latex "$linuxdoc_to_latex_command" ""
\\converter linuxdoc dvi "$linuxdoc_to_dvi_command" ""
\\converter linuxdoc html "$linuxdoc_to_html_command" ""
\\converter docbook dvi "$docbook_to_dvi_command" ""
\\converter docbook html "$docbook_to_html_command" ""

\\converter latex lyx "$tex_to_lyx_command" ""
\\converter literate lyx "$literate_to_lyx_command" ""
\\converter html latex "$html_to_latex_command" ""
\\converter word latex "$word_to_latex_command" ""

\\converter gif  eps "$gif_to_eps" ""
\\converter png  eps "$png_to_eps" ""
\\converter jpg  eps "$jpg_to_eps" ""

\\converter gif  png "$gif_to_png" ""
\\converter eps  png "$eps_to_png" ""
\\converter epsi png "$eps_to_png" ""
\\converter jpg  png "$jpg_to_png" ""

\\converter gif  xpm "$gif_to_xpm" ""
\\converter eps  xpm "$eps_to_xpm" ""
\\converter epsi xpm "$eps_to_xpm" ""
\\converter jpg  xpm "$jpg_to_xpm" ""
\\converter png  xpm "$png_to_xpm" ""
 
\\converter eps  pdf "$eps_to_pdf" ""
\\converter epsi pdf "$eps_to_pdf" ""

\\viewer dvi "$DVI_VIEWER"
\\viewer html "$HTML_VIEWER"
\\viewer pdf "$PDF_VIEWER"
\\viewer ps "$GHOSTVIEW -swap"
\\viewer eps "$GHOSTVIEW"

\\ps_command "$GS"
\\ascii_roff_command "$ascii_roff_command"
\\chktex_command "$chktex_command"
\\spell_command "$SPELL"
dnl \\fax_command "$fax_command"
\\print_spool_command "$print_spool_command"
\\print_spool_printerprefix "$print_spool_printerprefix"
\\font_encoding "$chk_fontenc"
EOF

# Remove superfluous files if we are not writing in the main lib
# directory 
for file in lyxrc.defaults textclass.lst packages.lst \
            doc/LaTeXConfig.lyx ; do
  # we rename the file first, so that we avoid comparing a file with itself
  mv $file $file.new
  if test -r $srcdir/$file && diff $file.new $srcdir/$file >/dev/null 2>/dev/null ; 
  then 
    echo "removing $file, which is identical to the system global version"
    rm -f $file.new
  else
    mv $file.new $file
  fi
done


# Final clean-up
if test $lyx_keep_temps = no ; then
rm -f chkconfig.sed chkconfig.vars wrap_chkconfig.* chklayouts.tex \
      missfont.log
fi
