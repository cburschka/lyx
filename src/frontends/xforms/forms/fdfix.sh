#! /bin/sh
#
# NOTE: This is NOT the same fdfix.sh as in ${top_srcdir}/forms
#       It is a modified version to suit use for gui-indep.
#

if [ ! -f $1 ]; then
    echo "Input file does not exist. Cannot continue"
    exit 1
fi

FDESIGN=fdesign
base=`basename $1 .fd`

if [ $1 = $base ]; then
    echo "Input file is not a .fd file. Cannot continue"
    exit 1
fi

cin=$base.c
cout=$base.C
hin=$base.h
hout=$base.H

classname=`echo $base | cut -c6-`
firstchar=`echo $classname | cut -c1 | tr a-z A-Z`
rest=`echo $classname | cut -c2-`
classname=Form$firstchar$rest

# Create .c and .h files
$FDESIGN -convert $1

# Modify .h file for use by LyX
echo "// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext" > $hout
sed -f fdfixh.sed < $hin >> $hout

# Patch the .h file if a patch exists
if [ -f "$hout.patch" ] ; then
    echo "Patching $hout with $hout.patch"
    patch -s $hout < $hout.patch
fi

# Modify .c file for use by LyX
echo "// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext" > $cout
echo "#include <config.h>" >> $cout
echo "#include \"lyx_gui_misc.h\"" >> $cout
echo "#include \"gettext.h\"" >> $cout
echo >> $cout

sed -f fdfixc.sed < $cin | sed -e "s/CLASSNAME/$classname/" >> $cout

# Patch the .C file if a patch exists
if [ -f "$cout.patch" ] ; then
    echo "Patching $cout with $cout.patch"
    patch -s $cout < $cout.patch
fi

# Clean up, to leave .C and .h files
rm -f $cin $hin
mv $hout $hin
