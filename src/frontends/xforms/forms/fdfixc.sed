# file fdfixc.sed
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
#
# Full author contact details are available in file CREDITS
#
# Use so:
# sed -f fdfixc.sed < somefile.c > fixedfile.C
#
# It contains the instructions that sed requires to manipulate
# the .c files produced by fdesign into a form usable by LyX

# Pretty formatting; remove trailing whitespace.
s/[ 	]*$//


# Delete blank lines and fdesign comments.
/^$/d
/generated with fdesign/d
/^\/\*-----/d


# Remove '#include "forms.h"'. It is replace by a macro in fdfix.sh.
/^#include \"forms\.h\"/d


# Pretty formatting; replace all leading whitespace with a tab
/^[ 	]/s/^[ 	]*/	/


# Rename struct "FD_form_xxx" as "FD_xxx"
s/FD_form_\(.*\)/FD_\1/


# Rename the function create_form_form_xxx(void) as build_xxx()
s/\(.*\) \*create_form_form_\(.*\)\([(]void[)]\)/\
\
\1 * build_\2(void * parent)/


# Pretty formatting
s/FL_OBJECT \*obj;/FL_OBJECT * obj;\
/

# Insert a line before each new FL_OBJECT
/obj = /i\

# Ditto for fl_bgn
/fl_bgn/i\

# Ditto for fl_end
/fl_end/i\


# Rewrite "fdui->form_xxx" as "fdui->form"
# xxx is followed by ' ', '->' or ')', so use these to flag when xxx ends.
/fdui->form/s/\(fdui->form\)_[^ )-]*/\1/


# We use new/delete not malloc/free so change to suit.
s/\(\(FD_[^ ]*\) \*fdui =\).*sizeof(\*fdui))/\1 new \2/


# We need to store a pointer to the dialog in u_vdata so that the
# callbacks will work.
s/\(	fdui->form\)\(.*bgn_form.*\)/\1\2\
\1->u_vdata = parent;/


# For all lines containing "bmtable",
# replace "fl_add_button" with "fl_add_bmtable"
/bmtable/ s/fl_add_button/fl_add_bmtable/


# For all lines containing "combox",
# replace "fl_add_choice" with "fl_add_combox"
# Note that only two combox types exist, whilst there are four choice types.
/combox/ {
s/fl_add_choice/fl_add_combox/
s/BROWSER/NORMAL/
s/CHOICE2/COMBOX/
s/CHOICE/COMBOX/
}


# For all lines containing fl_add_choice,
# add a line that turns off the title. (These titles can contain meta-chars
# that just look nasty ;-)
/fl_add_choice/a\
	fl_set_choice_notitle(obj, 1);


# For all lines containing "fl_" and a string _not_ containing |,
# replace the string with _(string)
/fl_/ s/".[^|]*"/_(&)/


#  For all lines containing "_shortcut" and a string containing |, 
#  replace the string with scex(_(string)).c_str()
/_shortcut/ s/".*[|].*"/scex(_(&)).c_str()/


# For all lines containing "fl_add" and a string containing |
# change so:
#        fdui->counter_zoom = obj = fl_add_counter(...,"Zoom %|#Z");
# becomes
#        c_str = _("Zoom %|#Z");
#        fdui->counter_zoom = obj = fl_add_counter(...,idex(c_str).c_str());
#        fl_set_button_shortcut(obj,scex(c_str).c_str(),1);

/fl_add.*".*[|].*"/s/fdui\(.*\)"\(.*\)".*/c_str = _("\2");\
	fdui\1idex(c_str).c_str());\
	fl_set_button_shortcut(obj,scex(c_str).c_str(),1);/


# gettext will get confused if the string contains a "%" unless the line is
# preceeded immediately by "// xgettext:no-c-format"
/_(".*[%].*")/i\
	// xgettext:no-c-format


# Someone got busy and put spaces in after commas but didn't allow for the
# autogeneration of the files so their pretty formatting got lost. Not anymore.
s/,\([^ ]\)/, \1/g

# Clean up one special case where a comma appears at the end of a string
# while ensuring "...", "..." isn't affected.
s/\("[^"]+,\) \("\)/\1\2/g
