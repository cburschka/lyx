#!/bin/sh
# A script to check whether there have been any string changes.

# The script expects an environment variable FARM that 
# will provide it with the location of the LyX www tree.

DEBUG="echo";

if [ -z "$FARM" ]; then
  echo "You must set the FARM variable to run this script, e.g.:";
  echo "# FARM=/cvs/lyx-www/ bash check-po.sh";
  exit 1;
fi

FARM=${FARM%/};
FARM="$FARM/farm/cookbook/LyX";
# Sanity check
if [ ! -f "$FARM/i18n.php" ]; then 
  echo "$FARM does not look like LyX's www tree!";
  exit 1;
fi

# Get us to the root of the tree we are in.
MYDIR=${0%check-po.sh};
if [ -n "$MYDIR" ]; then
  cd $MYDIR;
fi
cd ../../;
LYXROOT=$(pwd);

# Are we in trunk or branch?
TRUNK="TRUE";
if ls status.* 2>/dev/null | grep -q status; then 
  TRUNK="";
fi

# Git or SVN?
VCS="";
if svn log >/dev/null 2>&1; then
  VCS="svn";
else if git diff >/dev/null 2>&1; then
  VCS="git";
fi

if [ -n "$VCS" ]; then 
  echo "Unable to determine version control system!";
  exit 1;
fi

# Sanity check
if ! cd po/; then 
  echo "Cannot cd to po/ directory!";
  pwd
  exit 1;
fi

echo Remerging...
make update-po >/dev/null 2>&1;
echo

echo Running make i18n.inc...
make i18n.inc  >/dev/null 2>&1;
if [ -n "$TRUNK" ]; then
  mv i18n.inc i18n_trunk.inc
  I18NFILE=i18n_trunk.inc;
else
  I18NFILE=i18n.inc;
fi

if diff -w -q $I18NFILE $FARM/$I18NFILE >/dev/null; then
  # No differences found
  echo No string differences found.
  if [ "$VCS" = "svn" ]; then
    svn revert *.po  >/dev/null 2>&1;
  else
    git co *.po >/dev/null 2>&1;
  fi
  exit 0;
fi

# else
if [ "$VCS" = "svn" ]; then
  $DEBUG svn ci *.po;
else
  $DEBUG git commit *.po -m "Remerge strings.";
  $DEBUG git svn dcommit;
fi

exit 1;

if ! cd $FARM; then
  echo "Unable to cd to $FARM!";
  exit 1;
fi

echo
echo Updating the www-user tree...
svn up

echo Copying $I18NFILE...;
cp $LYXROOT/po/$I18NFILE .;

echo Committing...;
svn commit -m "* $I18NFILE: update stats" $I18NFILE;

echo DONE!
