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


# Be Bourne compatible (taken from autoconf 2.53)
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
  emulate sh
  NULLCMD=:
elif test -n "${BASH_VERSION+set}" && (set -o posix) >/dev/null 2>&1; then
  set -o posix
fi



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
     tmpfname="/tmp/x$$.ltx";
     echo "\\documentstyle{article}" > $tmpfname
     echo "\\begin{document}\\end{document}" >> $tmpfname
     inpname=`cygpath -w $tmpfname | tr '\\\\' /`
     echo "\\input{$inpname}" > wrap_temp$$.ltx
     check_err=`latex wrap_temp$$.ltx 2>&1 < /dev/null | grep Error`
     rm -f wrap_temp$$.* /tmp/x$$.*
     if [ x"$check_err" = "x" ]
     then
       echo "configure: cygwin detected; path correction"
       srcdir=`cygpath -w "${srcdir}" | tr '\\\\' /`
       echo "srcdir=${srcdir}"
     else
       echo "configure: cygwin detected; path correction is not needed"
     fi
     ;;
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
SEARCH_PROG([for a LaTeX2e program],LATEX,pplatex latex2e latex,CHECKLATEX2E,dnl
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

SEARCH_PROG([for a TGIF viewer and editor], TGIF_EDITOR, tgif)
TGIF_VIEWER="$TGIF_EDITOR"

SEARCH_PROG([for a FIG viewer and editor], FIG_EDITOR, xfig)
FIG_VIEWER="$FIG_EDITOR"

SEARCH_PROG([for a FEN viewer and editor], FEN_EDITOR, xboard)
test "$FEN" = "xboard" && FEN_EDITOR="xboard -lpf \$\$i -mode EditPosition"
FEN_VIEWER="$FEN_EDITOR"

SEARCH_PROG([for a raster image viewer], RASTERIMAGE_VIEWER, xv kview gimp)

SEARCH_PROG([for a raster image editor], RASTERIMAGE_EDITOR, gimp)

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

# Search something to preview postscript
SEARCH_PROG([for a Postscript previewer],GHOSTVIEW,gsview32 gv ghostview)
case $GHOSTVIEW in
  gv|ghostview) PS_VIEWER="$GHOSTVIEW -swap" ;;
  *) PS_VIEWER="$GHOSTVIEW";;
esac
EPS_VIEWER=$GHOSTVIEW

# Search for a program to preview pdf
SEARCH_PROG([for a PDF preview],PDF_VIEWER, acrobat acrord32 gsview32 acroread gv ghostview xpdf)

# Search something to preview dvi
SEARCH_PROG([for a DVI previewer],DVI_VIEWER, xdvi windvi yap)

# Search something to preview html
SEARCH_PROG([for a HTML previewer],HTML_VIEWER, "mozilla file://\$\$p\$\$i" netscape)

# Search for a program to convert ps to pdf
SEARCH_PROG([for a PS to PDF converter],ps_to_pdf_command,ps2pdf)
test $ps_to_pdf_command = "ps2pdf" && ps_to_pdf_command="ps2pdf -dCompatibilityLevel=1.3 \$\$i"

# Search for a program to convert dvi to ps
SEARCH_PROG([for a DVI to PS converter],dvi_to_ps_command,dvips)
test $dvi_to_ps_command = "dvips" && dvi_to_ps_command="dvips -o \$\$o \$\$i"

# Search for a program to convert dvi to pdf
SEARCH_PROG([for a DVI to PDF converter],dvi_to_pdf_command,dvipdfm)
test $dvi_to_pdf_command = "dvipdfm" && dvi_to_pdf_command="dvipdfm \$\$i"

# We have a script to convert previewlyx to ppm or to png
lyxpreview_to_bitmap_command='python $$s/lyxpreview2bitmap.py'

# Search for 'dvipng'. Only enable the conversion from lyxpreview -> png
# if dvipng is found.
SEARCH_PROG([for dvipng],DVIPNG,dvipng)

test "$DVIPNG" = "dvipng" && \
	lyxpreview_to_png_command = $lyxpreview_to_bitmap_command

# Search a *roff program (used to translate tables in ASCII export)
LYXRC_PROG([for a *roff formatter], \ascii_roff_command, dnl
  'groff -t -Tlatin1 $$FName' nroff,dnl
  test "$prog" = "nroff" && prog='tbl $$FName | nroff')

# Search the ChkTeX program
LYXRC_PROG([for ChkTeX], \chktex_command,dnl
   "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38")

# Search for a spellchecker
LYXRC_PROG([for a spell-checker], \spell_command, ispell)

# Search for computer algebra systems
SEARCH_PROG([for Octave],OCTAVE,octave)
SEARCH_PROG([for Maple],MAPLE,maple)

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
SEARCH_PROG([for a fax program], fax_command, kdeprintfax ksendfax)
test $fax_command != "none" && fax_command="$fax_command \$\$i"

# Search for LinuxDoc support
SEARCH_PROG([for SGML-tools 1.x (LinuxDoc)], LINUXDOC, sgml2lyx)
if test $LINUXDOC != none; then
  chk_linuxdoc=yes
  bool_linuxdoc=true
  linuxdoc_cmd="\\def\\haslinuxdoc{yes}"
else
  chk_linuxdoc=no
  bool_linuxdoc=false
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
if test $DOCBOOK != none; then
  chk_docbook=yes
  bool_docbook=true
  docbook_cmd="\\def\\hasdocbook{yes}"
else
  chk_docbook=no
  bool_docbook=false
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
  MSG_CHECKING(list of textclasses,+)
  cat >textclass.lst <<EOF
# This file declares layouts and their associated definition files
# (include dir. relative to the place where this file is).
# It contains only default values, since chkconfig.ltx could not be run
# for some reason. Run ./configure if you need to update it after a
# configuration change.
EOF
  # build the list of available layout files and convert it to commands
  # for chkconfig.ltx
  for file in ./layouts/*.layout ${srcdir}/layouts/*.layout ; do
    case $file in
      */\*.layout) ;;
      *) if test -r "$file" ; then
           class=`echo $file | sed -e 's%^.*layouts/\(.*\)\.layout$%\1%'`
           cleanclass=`echo $class | tr ' -' '__'`
changequote([,])dnl
           # make sure the same class is not considered twice
           if test  x`[eval] echo $ac_n '${found_'$cleanclass'}'` = x ; then
             [eval] "found_$cleanclass=yes"
changequote(,)dnl
	     # The sed commands below are a bit scary. Here is what they do:
	     # 1-3: remove the \DeclareFOO macro and add the correct boolean 
	     #      at the end of the line telling whether the class is 
             #      available
	     # 4: if the macro had an optional argument with several 
	     #    parameters, only keep the first one
	     # 5: if the macro did not have an optional argument, provide one 
	     #    (equal to the class name)
	     # 6: remove brackets and replace with correctly quoted entries
	     grep '\\Declare\(LaTeX\|DocBook\|LinuxDoc\)Class' "$file" \
	      | sed -e 's/^.*\DeclareLaTeXClass *\(.*\)/\1 "false"/' \
		    -e 's/^.*\DeclareDocBookClass *\(.*\)/\1 "'$bool_docbook'"/' \
		    -e 's/^.*\DeclareLinuxDocClass *\(.*\)/\1 "'$bool_linuxdoc'"/' \
		    -e 's/\[\([^,]*\),[^]]*\]/[\1]/' \
		    -e 's/^{/['$class']{/' \
		    -e 's/\[\([^]]*\)\] *{\([^}]*\)}/"'$class'" "\1" "\2"/' \
                    >>textclass.lst
           fi
	 fi ;;
    esac
  done 
  MSG_RESULT(done)
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
\\Format asciichess asc    "ASCII (chess output)"  "" ""	""
\\Format asciiimage asc    "ASCII (image)"         "" ""	""
\\Format asciixfig  asc    "ASCII (xfig output)"   "" ""	""
\\Format agr        agr     GRACE                  "" ""	""
\\Format bmp        bmp     BMP                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format date       ""     "date command"          "" ""	""
\\Format dateout    tmp    "date (output)"         "" ""	""
\\Format docbook    sgml    DocBook                B  ""	""
\\Format docbook-xml xml    "Docbook (xml)"        "" "" ""
\\Format dvi        dvi     DVI                    D  "$DVI_VIEWER"	""
\\Format eps        eps     EPS                    "" "$EPS_VIEWER"	""
\\Format fax        ""      Fax                    "" ""	""
\\Format fen        fen     FEN                    "" "$FEN_VIEWER"	"$FEN_EDITOR"
\\Format fig        fig     XFig                   "" "$FIG_VIEWER"	"$FIG_EDITOR"
\\Format gif        gif     GIF                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format html       html    HTML                   H  "$HTML_VIEWER"	""
\\Format jpg        jpg     JPG                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format latex      tex     LaTeX                  L  ""	""
\\Format linuxdoc   sgml    LinuxDoc               x  ""	""
\\Format lyx        lyx     LyX                    "" ""	""
\\Format lyxpreview lyxpreview "LyX Preview"       "" ""	""
\\Format literate   nw      NoWeb                  N  ""	""
\\Format pbm        pbm     PBM                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format pdf        pdf    "PDF (ps2pdf)"          P  "$PDF_VIEWER"	""
\\Format pdf2       pdf    "PDF (pdflatex)"        F  "$PDF_VIEWER"	""
\\Format pdf3       pdf    "PDF (dvipdfm)"         m  "$PDF_VIEWER"	""
\\Format pdftex     pdftex_t PDFTEX                "" ""	""
\\Format pgm        pgm     PGM                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format png        png     PNG                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format ppm        ppm     PPM                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format program    ""      Program                "" ""	""
\\Format ps         ps      Postscript             t  "$PS_VIEWER"	""
\\Format pstex      pstex_t PSTEX                  "" ""	""
\\Format text       txt     ASCII                  A  ""	""
\\Format textparagraph txt "ASCII (paragraphs)"    "" ""	""
\\Format tgif       obj     TGIF                   "" "$TGIF_VIEWER"	"$TGIF_EDITOR"
\\Format tiff       tif     TIFF                   "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format word       doc     Word                   W  ""	""
\\Format xbm        xbm     XBM                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"
\\Format xpm        xpm     XPM                    "" "$RASTERIMAGE_VIEWER"	"$RASTERIMAGE_EDITOR"

\\converter date       dateout    "date +%d-%m-%Y > \$\$o"	""
\\converter docbook    docbook-xml "cp \$\$i \$\$o" "xml"
\\converter docbook    dvi        "$docbook_to_dvi_command"	""
\\converter docbook    html       "$docbook_to_html_command"	""
\\converter dvi        pdf3       "$dvi_to_pdf_command"	""
\\converter dvi        ps         "$dvi_to_ps_command"	""
\\converter fen        asciichess "python \$\$s/fen2ascii.py \$\$i \$\$o"	""
\\converter fig        pdftex     "sh \$\$s/fig2pdftex.sh \$\$i \$\$o"	""
\\converter fig        pstex      "sh \$\$s/fig2pstex.sh \$\$i \$\$o"	""
\\converter html       latex      "$html_to_latex_command"	""
\\converter latex      html       "$latex_to_html_command"	"originaldir,needaux"
\\converter latex      dvi        "$latex_to_dvi"	"latex"
\\converter latex      lyx        "$tex_to_lyx_command"	""
\\converter latex      pdf2       "$latex_to_pdf"	"latex"
\\converter linuxdoc   dvi        "$linuxdoc_to_dvi_command"	""
\\converter linuxdoc   html       "$linuxdoc_to_html_command"	""
\\converter linuxdoc   latex      "$linuxdoc_to_latex_command"	""
\\converter linuxdoc   lyx        "$linuxdoc_to_lyx_command"	""
\\converter literate   latex      "$literate_to_tex_command"	""
\\converter literate   lyx        "$literate_to_lyx_command"	""
\\converter lyxpreview png        "$lyxpreview_to_png_command"	""
\\converter lyxpreview ppm        "$lyxpreview_to_bitmap_command"	""
\\converter ps         fax        "$fax_command"	""
\\converter ps         pdf        "$ps_to_pdf_command"	""
\\converter word       latex      "$word_to_latex_command"	""
EOF

### the graphic converter part with the predefined ones
#### Search for the nonstandard converting progs
#
SEARCH_PROG([for an FIG -> EPS/PPM converter], FIG2DEV, fig2dev)
if test "$FIG2DEV" = "fig2dev"; then
cat >>$outfile <<EOF
\\converter fig        eps        "fig2dev -L eps \$\$i \$\$o" ""
\\converter fig        ppm        "fig2dev -L ppm \$\$i \$\$o" ""
\\converter fig        png        "fig2dev -L png \$\$i \$\$o" ""
EOF
fi

SEARCH_PROG([for an TIFF -> PS converter], TIFF2PS, tiff2ps)
if test "$TIFF2PS" = "tiff2ps"; then
cat >>$outfile <<EOF
\\converter tiff        eps         "tiff2ps \$\$i > \$\$o" ""
EOF
fi

SEARCH_PROG([for an TGIF -> EPS/PPM converter], TGIF, tgif)
if test "$TGIF" = "tgif"; then
cat >>$outfile <<EOF
\\converter tgif       eps        "tgif -stdout -print -color -eps \$\$i > \$\$o" ""
\\converter tgif       pdf        "tgif -stdout -print -color -pdf \$\$i > \$\$o" ""
EOF
fi

SEARCH_PROG([for an EPS -> PDF converter], EPSTOPDF, epstopdf)
if test "$EPSTOPDF" = "epstopdf"; then
cat >>$outfile <<EOF
\\converter eps        pdf        "epstopdf --outfile=\$\$o \$\$i" ""
EOF
fi

#### Add Grace conversions (xmgrace needs an Xserver, gracebat doesn't.)
SEARCH_PROG([for a Grace -> Image converter], GRACE, gracebat)
if test "$GRACE" = "gracebat"; then
cat >>$outfile <<EOF
\\converter agr        eps        "gracebat -hardcopy -printfile \$\$o -hdevice EPS \$\$i 2>/dev/null" ""
\\converter agr        png        "gracebat -hardcopy -printfile \$\$o -hdevice PNG \$\$i 2>/dev/null" ""
\\converter agr        jpg        "gracebat -hardcopy -printfile \$\$o -hdevice JPEG \$\$i 2>/dev/null" ""
\\converter agr        ppm        "gracebat -hardcopy -printfile \$\$o -hdevice PNM \$\$i 2>/dev/null" ""
EOF
fi

cat >>$outfile <<EOF

$rc_entries
\\font_encoding "$chk_fontenc"
EOF

######## X FONTS
# create a fonts.dir file to make X fonts available to LyX
echo "checking for TeX fonts"

fontlist="cmex10 cmmi10 cmr10 cmsy10 eufm10 msam10 msbm10 wasy10"
rm -f xfonts/fonts.dir xfonts/fonts.scale xfonts/tmpfonts

num=0
touch xfonts/tmpfonts
for font in $fontlist ; do
  MSG_CHECKING(for $font,+)
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
  # create a resource list file for Display Postscript
  (cd xfonts ; rm -f PSres.upr ; makepsres) 2>/dev/null || true
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
if test ! -r xfonts/fonts.dir ; then
    echo "removing font links"
    rm -f xfonts/*.pfb xfonts/fonts.scale
fi

# Final clean-up
if test $lyx_keep_temps = no ; then
rm -f chkconfig.sed chkconfig.vars wrap_chkconfig.* chklayouts.tex \
      missfont.log
fi
