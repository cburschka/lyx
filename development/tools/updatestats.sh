#!/bin/sh
# A script to update the LyX i18n.inc statistics

# The script expects an environment variable FARM that 
# will provide it with the location of the LyX www tree.

if [ -z "$FARM" ]; then
  echo "You must set the FARM variable to run this script, e.g.:";
  echo "# FARM=/cvs/lyx-www/ bash updatestats.sh";
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
MYDIR=${0%updatestats.sh};
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

# Sanity check
if ! cd po/; then 
  echo "Cannot cd to po/ directory!";
  pwd
  exit 1;
fi

echo Running make i18n.inc...
make i18n.inc;
if [ -n "$TRUNK" ]; then
  mv i18n.inc i18n_trunk.inc
  I18NFILE=i18n_trunk.inc;
else
  I18NFILE=i18n.inc;
fi

if ! cd $FARM; then
  echo "Unable to cd to $FARM!";
  exit 1;
fi

# Are we under svn or git?
GIT="";
if [ -d .git/ ]; then GIT="TRUE"; fi

echo
echo Updating the www-user tree...
svn up

echo Copying $I18NFILE...;
cp $LYXROOT/po/$I18NFILE .;

echo Committing...;
if [ -z "$GIT" ]; then
  svn commit -m "* $I18NFILE: update stats" $I18NFILE;
else
  git commit -m "* $I18NFILE: update stats" $I18NFILE;
  git svn dcommit;
fi

echo DONE!
