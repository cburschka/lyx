#! /bin/sh

if [ "$1" = "$2" ]; then
    echo "Input and Output file can not be the same."
    exit 1
fi

if [ -f $2 ]; then
	echo "Output file already exists, overwrite?"
	read
	if [ "$REPLY" != "y" ];	then
	    exit 0
	fi
fi

if [ ! -f $1 ]; then
    echo "Input file does not exist, can not continue"
    exit 1
fi

# If there is a patch for the outputfile patch the input file with it.
if [ -f "$2.patch" ]; then
    echo "Patching $1 with $2.patch"
    patch -s $1 < "$2.patch"
fi

echo "// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext" > $2
echo "#include <config.h>" >> $2
echo "#include \"lyx_gui_misc.h\"" >> $2
echo "#include \"gettext.h\"" >> $2
echo >> $2

# The commands to sed does this:
#
# -e 's/#include "forms\.h"/#include FORMS_H_LOCATION/'
#
#  Replace "forms.h" by FORMS_H_LOCATION in #include directives. This
#  macro is defined in config.h and is either <forms.h> or
#  <X11/forms.h>. 
#
# -e '/fl_/ s/".[^|]*"/_(&)/'
#  
#  For all lines containing "fl_" and a string _not_ containging |,
#  replace the string with _(string)
#
# -e '/shortcut/ s/".*[|].*"/scex(_(&))/'
#
#  For all lines containing "shortcut" and a string containing |, replace
#  the string with scex(_(string))
#
# -e '/fl_add/ s/".*[|].*"/idex(_(&))/'
#  For all lines containing "fl_add" and a string containing |, replace
#  the string with idex(_(string))
#
# -e '/fl_add/ s/idex("\(.*\)").*$/&fl_set_button_shortcut(obj,"\1",1);/'
# For all lines containing "fl_add" and a string containing |, add the
# shortcut command after the end of this line
#
# -e 's/fl_set_object_lcolor/fl_set_object_lcol/' 
#
#  For all lines replace "fl_set_object_lcolor" with "fl_set_object_lcol"
#  This will be removed when we don't support 0.81
#
# -e 's/fdui->.*->fdui = fdui/\/\/&/'
#
#  For all lines replace "fdui->...->fdui" with "//fdui->...->fdui"
#  This will be removed when we don't support 0.81
#
# -e 's/\(\(FD_[^ ]*\) \*fdui.*\)sizeof(\*fdui)/\1sizeof(\2)/'
#
#  Some picky/broken compilers (eg AIX's xlC) don't like evaluating  
#  sizeof(*fdui) before fdui has been given a value and output a warning. 
#  This will not be needed anymore as soon as fdesign is fixed (already 
#  reported to its authors).


cat $1 | sed \
-e 's/#include "forms\.h"/#include FORMS_H_LOCATION/' \
-e '/fl_/ s/".[^|]*"/_(&)/' \
-e '/shortcut/ s/".*[|].*"/scex(_(&))/' \
-e '/fl_add/ s/".*[|].*"/idex(_(&))/' \
-e '/fl_add/ s/idex(\(.*\)").*$/&fl_set_button_shortcut(obj,scex(\1")),1);/' \
-e 's/fl_set_object_lcolor/fl_set_object_lcol/' \
-e 's/fdui->.*->fdui = fdui/\/\/&/' \
-e 's/\(\(FD_[^ ]*\) \*fdui.*\)sizeof(\*fdui)/\1sizeof(\2)/' >> $2
