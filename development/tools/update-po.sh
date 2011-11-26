#!/bin/sh
# A script to check whether there have been any string changes.
# If it finds some, it commits the new po files and then updates
# the stats.

# The script expects an environment variable FARM that will provide 
# it with the location of the LyX www tree.

DEBUG="";
COMMIT="";

while getopts ":cdh" options $ARGS; do
  case $options in
    c)  COMMIT="TRUE";;
    d)  DEBUG="echo";;
    h)  echo "update-po.sh [-c] [-d]"; 
        echo "-c: Commit any changes we find.";
        echo "-d: Debugging mode.";
        echo "You must also point the FARM variable to LyX's www tree."; 
        exit 0;;
  esac
done

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
MYDIR=${0%update-po.sh};
if [ -n "$MYDIR" ]; then
  if ! cd $MYDIR; then
    echo "Couldn't cd to $MYDIR!";
    exit 1;
  fi
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
if svn log -l 5 >/dev/null 2>&1; then
  VCS="svn";
elif git diff >/dev/null 2>&1; then
  VCS="git";
  # We need to make sure that we have a tree without any unpushed 
  # commits. Otherwise git svn dcommit would commit more than we
  # want.
  if git status | grep -Pq 'Your branch is (?:ahead|behind)'; then
    echo "Your git tree is not clean. Please correct the situation and re-run.";
    echo;
    git status;
    exit 10;
  fi
fi

if [ -z "$VCS" ]; then 
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

if [ -n "$TRUNK" ]; then
  I18NFILE=i18n_trunk.inc;
else
  I18NFILE=i18n.inc;
fi

# make sure things are clean
rm -f i18n.inc;
svn revert $FARM/$I18NFILE;

echo Running make i18n.inc...
make i18n.inc  >/dev/null 2>&1;
if [ -n "$TRUNK" ]; then
  mv -f i18n.inc i18n_trunk.inc
fi

if diff -w -q $I18NFILE $FARM/$I18NFILE >/dev/null 2>&1; then
  echo No string differences found.
  # So we will revert the changes to po files, which are probably
  # just dates and such.
  if [ "$VCS" = "svn" ]; then
    svn revert *.po;
  else
    git checkout *.po;
  fi
  exit 0;
fi

# So there are differences.

if [ -z "$COMMIT" ]; then
  echo "Differences found!";
  diff -w $I18NFILE $FARM/$I18NFILE | less;
  if [ "$VCS" = "svn" ]; then
    svn revert *.po;
  else
    git checkout *.po;
  fi
  exit 0;
fi

if [ "$VCS" = "svn" ]; then
  $DEBUG svn ci *.po;
else
  $DEBUG git commit *.po -m "Remerge strings.";
  $DEBUG git svn dcommit;
fi

echo

if ! cd $FARM; then
  echo "Unable to cd to $FARM!";
  exit 1;
fi

echo Updating the www-user tree...
# note that we're assuming this one is svn.
svn up;

echo Moving $I18NFILE...;
mv $LYXROOT/po/$I18NFILE .;

echo Committing...;
$DEBUG svn commit -m "* $I18NFILE: update stats" $I18NFILE;

if [ -n "$NOTSAFE" ]; then
  echo
  echo "Your LyX tree was not clean.";
  echo "Your will need to push changes to po files manually."
fi
