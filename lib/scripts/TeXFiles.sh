#!/bin/sh
# file: ~/bin/TeXFiles.sh
# all files		-> without option
# TeX class files	-> option cls 
# TeX style files 	-> option sty
# bibtex style files 	-> option bst
#
# with the help
# of kpsewhich and creates a 
# bstFiles.lst, clsFiles.lst, styFiles.lst
# without any parameter all files are created.
#
# Herbert Voss <voss@perce.org>
#
# Updates from Jean-Marc Lasgouttes.
#
CLS_STYLEFILE=clsFiles.lst
STY_STYLEFILE=styFiles.lst
BST_STYLEFILE=bstFiles.lst
version='$Id: TeXFiles.sh,v 0.2 2001-10-15'
progname=`echo $0 | sed 's%.*/%%'`
usage="Usage: TeXFiles.sh [-version | cls | sty | bst] 
       Default is without any Parameters,
       so that all files will be created"

types=$1
test -z "$types" && types="cls sty bst"

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
   *)   echo "ERROR: unknown type $type"
        exit 1;;
  esac

  rm -f $outfile
  touch $outfile

  dirs=`kpsewhich --show-path=$kpsetype 2>/dev/null | tr "$SEP" " " | sed -e 's%///%/%' -e 's%//%/%g' -e 's%!!%%g'`

  for dir in $dirs ; do
    find $dir -follow -name "*.$type" >>$outfile 2>/dev/null
  done

done
#echo "list saved in $STYLEFILE"
#echo `wc -l $CLS_STYLEFILE`		# only for information
#
# this is the end my friends ... Jim Morrison and the Doors in "The End"
