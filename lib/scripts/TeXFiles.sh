#!/bin/sh
# file: ~/bin/TeXFiles.sh
# all files		-> without option
# TeX class files	-> option cls 
# TeX style files 	-> option sty
# bibtex style files 	-> option bst
# bibtex database files -> option bib
#
# with the help
# of kpsewhich and creates a 
# bstFiles.lst, clsFiles.lst, styFiles.lst, bibFiles.lst
# without any parameter all files are created.
#
# Herbert Voss <voss@perce.org>
#
# Updates from Jean-Marc Lasgouttes.
#
# bib support added by Juergen Spitzmueller (v0.3)
#
CLS_STYLEFILE=clsFiles.lst
STY_STYLEFILE=styFiles.lst
BST_STYLEFILE=bstFiles.lst
BIB_FILES=bibFiles.lst
version='$Id: TeXFiles.sh,v 0.3 2003-02-14'
progname=`echo $0 | sed 's%.*/%%'`
usage="Usage: TeXFiles.sh [-version | cls | sty | bst | bib ] 
       Default is without any Parameters,
       so that all files will be created"

types=$1
test -z "$types" && types="cls sty bst bib"

#
# MS-DOS and MS-Windows define $COMSPEC or $ComSpec and use ';' to separate
# directories in path lists whereas Unixes uses ':'.
# $SEP holds the right character to be used by the scripts.
#
#???????????????
# never used this one with windows and what happens with mac??
#???????????????
#
if test -z "$COMSPEC" && test -z "$ComSpec"; then SEP=':'; else SEP=';'; fi

#
# A copy of some stuff from mktex.opt, so we can run in the presence of
# terminally damaged ls-R files.
#
if test "x$1" = x--help || test "x$1" = x-help; then
  echo "$usage"
  exit 0
elif test "x$1" = x--version || test "x$1" = x-version; then
  echo "`basename $0` $version"
  kpsewhich --version
  exit 0
fi

for type in $types ; do
  echo "Indexing files of type $type"
  case $type in 
   cls) outfile=$CLS_STYLEFILE
        kpsetype=.tex;;
   sty) outfile=$STY_STYLEFILE
        kpsetype=.tex;;
   bst) outfile=$BST_STYLEFILE
        kpsetype=.bst;;
   bib) outfile=$BIB_FILES
        kpsetype=.bib;;
   *)   echo "ERROR: unknown type $type"
        exit 1;;
  esac

  rm -f $outfile
  touch $outfile

  dirs=`kpsewhich --show-path=$kpsetype 2>/dev/null | tr "$SEP" " " | sed -e 's%/\{2,\}%/%g' -e 's%!!%%g'`

  for dir in $dirs ; do
    find $dir -follow -name "*.$type" >>$outfile 2>/dev/null
  done

done
#echo "list saved in $STYLEFILE"
#echo `wc -l $CLS_STYLEFILE`		# only for information
#
# this is the end my friends ... Jim Morrison and the Doors in "The End"
