#!/bin/bash

if [ -z "$BASH_VERSION" ]; then
	# http://article.gmane.org/gmane.editors.lyx.devel/159697
	echo "You must use bash to run this script";
	exit 1;
fi

function do_convert {
	for i in *; do 
		if [ ! -f $i ]; then continue; fi
		cp $i $i.old; 
		python $lyxdir/lib/scripts/prefs2prefs.py -l <$i.old >$i; 
	done
}

# find out where we are relative to the program directory
curdir=$(pwd);
progloc=$0;
pathto=${progloc%/*};
# get us into development/tools
if [ "$progloc" != "$pathto" ]; then
	if ! cd $pathto; then
	echo "Couldn't get to development/tools!";
	exit 1;
	fi
fi

curdir=`pwd`;
lyxdir=${curdir%/development/tools*};

if ! cd $lyxdir/lib/ui/; then
	echo "Couldn't get to lib/ui!";
	exit 1;
fi

do_convert;

if ! cd $lyxdir/lib/bind/; then
	echo "Couldn't get to lib/bind!";
	exit 1;
fi

do_convert;

#now do the subdirectories
for d in *; do 
	if [ ! -d $d ]; then continue; fi
	cd $d;
	do_convert;
	cd ..;
done
