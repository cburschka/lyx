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
CLS_STYLEFILE=clsFiles.lst
STY_STYLEFILE=styFiles.lst
BST_STYLEFILE=bstFiles.lst
version='$Id: TeXFiles.sh,v 0.1 2001-10-01'
progname=`echo $0 | sed 's%.*/%%'`
echo "$version"
usage="Usage: TeXFiles.sh [-version | cls | sty | bst] 
       Default is without any Parameters,
       so that all files will be created"

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
# Add the location of the script to the PATH if necessary.  This must
# be done before kpsewhich can be called, and thus cannot be put into
# mktex.opt
#
dirname=`echo $0 | sed 's%/*[^/][^/]*$%%'`
echo "Actual Dir: $dirname"
case $dirname in			# $dirname correct?
  "") 					# Do nothing
      ;;
  /* | [A-z]:/*) 			# / -> Absolute name 
      PATH="$dirname$SEP$PATH" ;;
   *)  					# other -> Relative name
      PATH="`pwd`/$dirname$SEP$PATH" ;;
esac					# end of case
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
#
# find the directories with kpsewhich. TeX has files ls-R to hold
# the tex-tree
#
test $# = 0 && {
  OIFS=$IFS; IFS=$SEP; set x `kpsewhich --show-path=ls-R 2>/dev/null`; shift; IFS=$OIFS
}
echo 'Delete old *files.lst, if present'
case "$1" in
    "cls")
	rm -f $CLS_STYLEFILE
	touch $CLS_STYLEFILE		# create new file
    ;;
    "sty")
	rm -f $STY_STYLEFILE
	touch $STY_STYLEFILE		# create new file
    ;;
    "bst")
	rm -f $BST_STYLEFILE
	touch $BST_STYLEFILE		# create new file
    ;;
    *) 					# all other
	rm -f $CLS_STYLEFILE $STY_STYLEFILE $BST_STYLEFILE
	touch $CLS_STYLEFILE $STY_STYLEFILE $BST_STYLEFILE 
    ;;
esac
echo "looking for all bst-style files in the latex tree";
for TEXMFLSR in "$@"; do		# go through the dirs
  case "$TEXMFLSR" in			# Prepend cwd if the directory was relative.
  "") continue ;;  			# it is an error if this case is taken.
  /* | [A-z]:/*) ;;			# leading /
  *)  TEXMFLSR="`pwd`/$TEXMFLS_R"	# all other
  esac
  echo "Dir: <$TEXMFLSR>"
case "$1" in				# list all files with suffix bst
    "cls")
	find $TEXMFLSR -follow -name *.cls >> $CLS_STYLEFILE
    ;;
    "sty")
	find $TEXMFLSR -follow -name *.sty >> $STY_STYLEFILE
    ;;
    "bst")
	find $TEXMFLSR -follow -name *.bst >> $BST_STYLEFILE
    ;;
    *) 
	find $TEXMFLSR -follow -name *.cls >> $CLS_STYLEFILE
	find $TEXMFLSR -follow -name *.sty >> $STY_STYLEFILE
	find $TEXMFLSR -follow -name *.bst >> $BST_STYLEFILE
    ;;
esac

  echo "done!"
done
#echo "list saved in $STYLEFILE"
#echo `wc -l $CLS_STYLEFILE`		# only for information
#
# this is the end my friends ... Jim Morrison and the Doors in "The End"
