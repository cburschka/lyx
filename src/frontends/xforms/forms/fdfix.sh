#! /bin/sh
#
# NOTE: This is NOT the same fdfix.sh as in ${top_srcdir}/forms
#       It is a modified version to suit use for gui-indep.
#
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

echo "// File modified by fdfix.sh for use by lyx (with xforms >= 0.86) and gettext" > $2
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
#  -e "/#include \"form_.*\"/a\\
#  #include \"$classname.h\" "
#
#   For all lines containing "#include "form_*"", append a line
#   containing the header file of the parent class
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
# -e 's/,\([^ ]\)/, \1/g'
#
# Someone got busy and put spaces in after commas but didn't allow for the
# autogeneration of the files so their pretty formatting got lost. Not anymore.
#
# -e 's/\(\(FD_[^ ]*\) \*fdui =\).*sizeof(\*fdui))/\1 dialog_ = new \2/'
#
# We use new/delete not malloc/free so change to suit.  Also the local
# variable for our dialog is called dialog_ so do that fix also.
#
#-e 's/\(FD_f\([^ ]*\)_\([^ ]*\)\) \*create[^ ]*/void F\2\3::build()/'
#
# Fixup the name of the create_form... function to have a signature almost
# matching that of the method it will become.  You just need to capitalize
# the forms name.
#
#   -e 's/\(fdui->form[^ ]*\)\(.*bgn_form.*\)/\1\2\
#     \1->u_vdata = this;/' \
#
# We need to store a pointer to the dialog in u_vdata so that the callbacks
# will work.
#

classname=`basename $1 .c | cut -c6-`
firstchar=`echo $classname | cut -c1 | tr a-z A-Z`
rest=`echo $classname | cut -c2-`
classname=Form$firstchar$rest
export classname

cat $1 | sed \
-e 's/#include \"forms\.h\"/#include FORMS_H_LOCATION/' \
-e "/#include \"form_.*\"/a\\
#include \"$classname.h\" " \
-e '/fl_/ s/".[^|]*"/_(&)/' \
-e '/shortcut/ s/".*[|].*"/scex(_(&))/' \
-e '/fl_add/ s/".*[|].*"/idex(_(&))/' \
-e '/fl_add/ s/idex(\(.*\)").*$/&fl_set_button_shortcut(obj,scex(\1")),1);/' \
-e 's/\(\(FD_[^ ]*\) \*fdui =\).*sizeof(\*fdui))/\1 new \2/' \
-e "s/\(FD_f\([^ _]*\)_\([^ ]*\)\) \*create_form_form[^ ]*/\1 * $classname::build_\3()/" \
-e 's/\(fdui->form[^ ]*\)\(.*bgn_form.*\)/\1\2\
  \1->u_vdata = this;/' \
-e 's/,\([^ ]\)/, \1/g' >> $2
