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
  IFS="${IFS=	}"; ac_save_ifs="$IFS"; IFS=":"
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
dnl LYXRC_VAR(VARIABLE_NAME, VALUE)
define(LYXRC_VAR,[dnl
rc_entries="$rc_entries
$1 \"$2\""])
dnl
dnl
dnl LYXRC_PROG(FEATURE-DESCRIPTION,VARIABLE-NAME,PROGRAMS-LIST,
dnl             ACTION-IF-FOUND,ACTION-IF-NOT-FOUND)
define(LYXRC_PROG,[dnl
SEARCH_PROG($1,prog,$3,$4,$5)
LYXRC_VAR($2,$prog)])
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
outfile=lyxrc.defaults
rc_entries=
lyx_check_config=yes
lyx_keep_temps=no
srcdir=
version_suffix=

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
  --with-version-suffix=suffix suffix of binary installed files
EOF
      exit 0;;
    --without-latex-config)
      lyx_check_config=no ;;
    --keep-temps)
      lyx_keep_temps=yes ;;
    --with-version-suffix*)
      version_suffix=`echo "$ac_option" | sed 's,--with-version-suffix=,,'`
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
case `uname -s` in
   CYGWIN*)
     echo "configure: cygwin detected; path correction"
     srcdir=`cygpath -w "${srcdir}" | tr '\\\\' /`
     echo "srcdir=${srcdir}" ;;
esac

#### Create the build directories if necessary
for dir in bind clipart doc examples help images kbd layouts reLyX \
    scripts templates ui xfonts; do
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
tex_to_lyx_command=`echo $tex_to_lyx_command | sed "s,reLyX,reLyX$version_suffix,"`

SEARCH_PROG([for a Noweb -> LyX converter],literate_to_lyx_command,noweb2lyx)
test $literate_to_lyx_command = "noweb2lyx" && literate_to_lyx_command="noweb2lyx \$\$i \$\$o"
literate_to_lyx_command=`echo $literate_to_lyx_command | sed "s,noweb2lyx,noweb2lyx$version_suffix,"`

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
LYXRC_PROG([for a Postscript interpreter], \ps_command, gs)

# Search something to preview postscript
SEARCH_PROG([for a Postscript previewer],GHOSTVIEW,gv ghostview)

# Search for a program to preview pdf
SEARCH_PROG([for a PDF preview],PDF_VIEWER,acroread gv ghostview xpdf)

# Search something to preview dvi
SEARCH_PROG([for a DVI previewer],DVI_VIEWER, xdvi windvi yap)

# Search something to preview html
SEARCH_PROG([for a HTML previewer],HTML_VIEWER, netscape)

# Search for a program to preview latex code
SEARCH_PROG([for a LaTeX preview],LATEX_VIEWER, "xterm -e less")

# Search for a program to convert ps to pdf
SEARCH_PROG([for a PS to PDF converter],ps_to_pdf_command,ps2pdf)
test $ps_to_pdf_command = "ps2pdf" && ps_to_pdf_command="ps2pdf \$\$i"

# Search for a program to convert dvi to ps
SEARCH_PROG([for a DVI to PS converter],dvi_to_ps_command,dvips)
test $dvi_to_ps_command = "dvips" && dvi_to_ps_command="dvips -o \$\$o \$\$i"

# Search for a program to convert dvi to pdf
SEARCH_PROG([for a DVI to PDF converter],dvi_to_pdf_command,dvipdfm)
test $dvi_to_pdf_command = "dvipdfm" && dvi_to_pdf_command="dvipdfm \$\$i"

# Search for a program to convert previewlyx to eps
SEARCH_PROG([for a LyX preview converter],lyxpreview_to_xpm_command,lyxpreview2xpm.sh)
test $lyxpreview_to_xpm_command = "lyxpreview2xpm.sh" && lyxpreview_to_xpm_command="lyxpreview2xpm.sh \$\$i \$\$o"

# Search a *roff program (used to translate tables in ASCII export)
LYXRC_PROG([for a *roff formatter], \ascii_roff_command, dnl
  'groff -t -Tlatin1 $$FName' nroff,dnl
  test "$prog" = "nroff" && prog='tbl $$FName | nroff')

# Search the ChkTeX program
LYXRC_PROG([for ChkTeX], \chktex_command,dnl
   "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38")

# Search for a spellchecker
LYXRC_PROG([for a spell-checker], \spell_command, ispell)

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
LYXRC_PROG([for a spool command], \print_spool_command, lp lpr)
case $prog in
  lp) print_spool_printerprefix="-d ";;
 lpr) print_spool_printerprefix="-P";;
   *) :;; # leave to empty values
esac
LYXRC_VAR(\print_spool_printerprefix, $print_spool_printerprefix)

SEARCH_PROG([for a LaTeX -> HTML converter], TOHTML, tth latex2html hevea)
latex_to_html_command=$TOHTML
case $TOHTML in
	tth) latex_to_html_command="tth -t -e2 -L\$\$b < \$\$i > \$\$o";;
 latex2html) latex_to_html_command="latex2html -no_subdir -split 0 -show_section_numbers \$\$i";;
      hevea) latex_to_html_command="hevea -s \$\$i";;
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
echo "creating textclass.lst"
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

echo "creating packages.lst"
PROVIDE_DEFAULT_FILE(packages.lst,dnl
[# This file should contain the list of LaTeX packages that have been
# recognized by LyX. Unfortunately, since configure could not find
# your LaTeX2e program, the tests have not been run. Run ./configure
# if you need to update it after a configuration change.
])
changequote(,)dnl

echo "creating doc/LaTeXConfig.lyx"
echo "s/@chk_linuxdoc@/$chk_linuxdoc/g" >> chkconfig.sed
echo "s/@chk_docbook@/$chk_docbook/g" >> chkconfig.sed
sed -f chkconfig.sed ${srcdir}/doc/LaTeXConfig.lyx.in >doc/LaTeXConfig.lyx

echo "creating $outfile"
cat >$outfile <<EOF
# This file has been automatically generated by LyX' lib/configure
# script. It contains default settings that have been determined by
# examining your system. PLEASE DO NOT MODIFY ANYTHING HERE! If you
# want to customize LyX, make a copy of the file LYXDIR/lyxrc as
# ~/.lyx/lyxrc and edit this file instead. Any setting in lyxrc will
# override the values given here.
\\Format text	  txt	ASCII		A
\\Format textparagraph txt ASCII(paragraphs)	""
\\Format docbook  sgml	DocBook		B
\\Format bmp      bmp	BMP		""
\\Format dvi	  dvi	DVI		D
\\Format eps	  eps	EPS		""
\\Format epsi	  epsi	EPSI		""
\\Format fax	  ""	Fax		""
\\Format fig	  fig	XFig		""
\\Format agr      agr	GRACE		""
\\Format html	  html	HTML		H
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
\\Format tgif     obj	TGIF		""
\\Format tiff     tif	TIFF		""
\\Format word	  doc	Word		W
\\Format xpm	  xpm	XPM		""
\\Format lyxpreview	  lyxpreview	LYXPREVIEW		""

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
\\converter lyxpreview xpm "$lyxpreview_to_xpm_command" ""

\\converter latex lyx "$tex_to_lyx_command" ""
\\converter literate lyx "$literate_to_lyx_command" ""
\\converter html latex "$html_to_latex_command" ""
\\converter word latex "$word_to_latex_command" ""

\\viewer dvi "$DVI_VIEWER"
\\viewer html "$HTML_VIEWER"
\\viewer pdf "$PDF_VIEWER"
\\viewer ps "$GHOSTVIEW -swap"
\\viewer eps "$GHOSTVIEW"
\\viewer latex "$LATEX_VIEWER"

$rc_entries
\\font_encoding "$chk_fontenc"
EOF

### the graphic converter part with the predefined ones
#### Search for tne nonstandard converting progs
#
SEARCH_PROG([for an FIG -> EPS/XPM converter], FIG2DEV, fig2dev)
if test "$FIG2DEV" = "fig2dev"; then
cat >>$outfile <<EOF
\\converter fig eps "fig2dev -L eps \$\$i \$\$o" ""
\\converter fig xpm "fig2dev -L xpm \$\$i \$\$o" ""
EOF
fi

SEARCH_PROG([for an TIFF -> PS converter], TIFF2PS, tiff2ps)
if test "$TIFF2PS" = "tiff2ps"; then
cat >>$outfile <<EOF
\\converter tiff eps "tiff2ps \$\$i > \$\$o" ""
EOF
fi

SEARCH_PROG([for an TGIF -> EPS/XPM converter], TGIF, tgif)
if test "$TGIF" = "tgif"; then
cat >>$outfile <<EOF
\\converter tgif eps "tgif -print -eps \$\$i" ""
\\converter tgif pdf "tgif -print -pdf \$\$i" ""
\\converter tgif png "tgif -print -png \$\$i" ""
\\converter tgif xpm "tgif -print -stdout -xpm \$\$i > \$\$o" ""
EOF
fi

SEARCH_PROG([for an EPS -> PDF converter], EPSTOPDF, epstopdf)
if test "$EPSTOPDF" = "epstopdf"; then
cat >>$outfile <<EOF
\\converter eps pdf "epstopdf --outfile=\$\$o \$\$i" ""
EOF
fi

SEARCH_PROG([for an EPSI ->  EPS/XPM converter], EPS2EPS, eps2eps)
if test "$EPS2EPS" = "eps2eps"; then
cat >>$outfile <<EOF
\\converter epsi pdf "epstopdf --outfile=\$\$o \$\$i" ""
\\converter epsi eps "eps2eps \$\$i $$o" ""
\\converter epsi xpm "eps2eps -sOutputFile=- \$\$i dummy.eps | convert - \$\$o" ""
EOF
fi

#### Add Grace conversions (xmgrace needs an Xserver, gracebat doesn't.)
SEARCH_PROG([for a Grace -> Image converter], GRACE, gracebat)
if test "$GRACE" = "gracebat"; then 
cat >>$outfile <<EOF
\\converter agr eps "gracebat -hardcopy -printfile \$\$o -hdevice EPS \$\$i 2>/dev/null" ""
\\converter agr png "gracebat -hardcopy -printfile \$\$o -hdevice PNG \$\$i 2>/dev/null" ""
\\converter agr xpm "gracebat -hardcopy -printfile - -hdevice PNG \$\$i 2>/dev/null | convert - \$\$o" ""
EOF
fi

######## X FONTS
# create a fonts.dir file to make X fonts available to LyX
echo "checking for TeX fonts"

fontlist="cmex10 cmmi10 cmr10 cmsy10 eufm10 msam10 msbm10 wasy10"
rm -f xfonts/fonts.dir xfonts/fonts.scale xfonts/tmpfonts

num=0
touch xfonts/tmpfonts
for font in $fontlist ; do
  MSG_CHECKING([for $font])
  result=no
  for ext in pfb pfa ttf ; do
    if filepath=`kpsewhich $font.$ext` ; then
      result="yes ($ext)"
      rm -f xfonts/$font.$ext
      ln -s $filepath xfonts 2>/dev/null
      echo "$font.$ext -unknown-$font-medium-r-normal--0-0-0-0-p-0-adobe-fontspecific" >>xfonts/tmpfonts
      num=`expr $num + 1`
      break
    fi
  done
  MSG_RESULT($result)
done

if test ! $num = 0 ; then
  echo $num >xfonts/fonts.scale
  cat xfonts/tmpfonts >>xfonts/fonts.scale
  cp xfonts/fonts.scale xfonts/fonts.dir
fi
rm -f xfonts/tmpfonts


# Remove superfluous files if we are not writing in the main lib
# directory
for file in $outfile textclass.lst packages.lst \
	    doc/LaTeXConfig.lyx xfonts/fonts.dir ; do
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
