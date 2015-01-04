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
  echo "# FARM=/cvs/lyx-www/ bash update-po.sh";
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

# We need to make sure that we have a tree without any unstaged 
# commits. Otherwise commit will fail.
if git status --porcelain -uno | grep -q .; then
  echo "Your git tree is not clean. Please correct the situation and re-run.";
  echo;
  git status --porcelain -uno;
  exit 10;
fi

# Are we in trunk or branch?
TRUNK="TRUE";
if ls status.* 2>/dev/null | grep -q status; then 
  TRUNK="";
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
  git checkout *.po;
  exit 0;
fi

# So there are differences.
if [ -z "$COMMIT" ]; then
  echo "Differences found!";
  diff -wu $FARM/$I18NFILE $I18NFILE | less;
  git checkout *.po *.gmo;
  exit 0;
fi

$DEBUG git commit *.po *.gmo -m "Remerge strings.";
COMMITS=$(git push -n 2>&1 | tail -n 1 | grep -v "Everything" | sed -e 's/^ *//' -e 's/ .*//');

if [ -z "$COMMITS" ]; then
  echo "We seem to be missing the commit of the po files!";
  exit 1;
fi

git log $COMMITS;

#Do we want to go ahead?
echo
echo "Do you want to push these commits?"
select answer in Yes No; do
  if [ "$answer" != "Yes" ]; then
    echo "You will need to push that commit manually, then.";
    break;
  else 
    git push;
    break;
  fi
done

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

