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

# Delete the fdesign comment
/generated with fdesign/d

# Rename struct FD_form_xxx as FD_xxx
s/FD_form_\(.*\)/FD_\1/

# In the struct declaration, we have renamed
#       FL_FORM * form_xxx;
# as
#       FL_FORM * form;
# so we must do so here too.
#
# 1. Rewrite "fdui->form_xxx->fdui = ..." as "fdui->form->fdui = ..."
s/fdui->form_\([^ ]*\)->fdui =/fdui->form->fdui =/

# 2. Rewrite "fdui->form_xxx = ..." as "fdui->form = ..."
s/fdui->form_\([^ ]*\) =/fdui->form =/

# 3. Rewrite "fdui->form_xxx->..." as "fdui->form->..."
s/fdui->form_\([^ ]*\)->/fdui->form->/

# Replace "forms.h" by FORMS_H_LOCATION in the #include directives
s/#include \"forms\.h\"/#include FORMS_H_LOCATION/

# Create the destructor.
# When the wrappers are here, delete from here
/\(.*\) *create_form_form/{
h
s/\(.*\) \*create_form_form[^ ]*/\1::~\1()/p
/\(.*\)::~\(.*\)()/{
i\
{\
  if ( form->visible ) fl_hide_form( form );\
  fl_free_form( form );\
}\

}
x
}
# When the wrappers are here, delete to here

# Rename the function create_form_form_xxx(void) as build_xxx()
s/\(.*\) \*create_form_form_\(.*\)\([(]void[)]\)/\1 * build_\2(void * parent)/

# We use new/delete not malloc/free so change to suit
s/\(\(FD_[^ ]*\) \*fdui =\).*sizeof(\*fdui))/\1 new \2/

# For all lines containing "bmtable",
# replace "fl_add_button" with "fl_add_bmtable"
/bmtable/ s/fl_add_button/fl_add_bmtable/

# For all lines containing "fl_" and a string _not_ containing |,
# replace the string with _(string)
/fl_/ s/".[^|]*"/_(&)/

#  For all lines containing "_shortcut" and a string containing |, 
#  replace the string with scex(_(string))
/_shortcut/ s/".*[|].*"/scex(_(&))/

# gettext will get confused if the string contains a "%" unless the line is
# preceeded immediately by // xgettext:no-c-format
/_(".*[%].*")/i\
  // xgettext:no-c-format

# For all lines containing "fl_add" and a string containing |
# do several things.
# Eg
#   fdui->counter_zoom = obj = fl_add_counter(FL_NORMAL_COUNTER,1,2,3,4,"Zoom %|#Z");
#
# becomes
#   fdui->counter_zoom = obj;
#   {
#     // xgettext:no-c-format
#     char const * const dummy = _("Zoom %|#Z");
#     fdui->counter_zoom = obj = fl_add_counter(FL_NORMAL_COUNTER,1,2,3,4,idex(dummy));
#     fl_set_button_shortcut(obj,scex(dummy),1);
#   }

/fl_add/{
/".*[|].*"/{

  s/fdui\(.*\)"\(.*\)".*/  {\
      char const * const dummy = _("\2");\
      fdui\1idex(dummy));\
      fl_set_button_shortcut(obj,scex(dummy),1);\
    }/

  /_(".*[%].*");/ s/\(.*\)\(char const [*]\)/\1\/\/ xgettext:no-c-format\
      \2/
}
}

# We need to store a pointer to the dialog in u_vdata so that the
# callbacks will work.
s/\(fdui->form\)\(.*bgn_form.*\)/\1\2\
  \1->u_vdata = parent;/

# Someone got busy and put spaces in after commas but didn't allow for the
# autogeneration of the files so their pretty formatting got lost. Not anymore.
#
s/,\([^ ]\)/, \1/g

# Clean up one special case where a comma appears at the end of a string
# while ensuring "...", "..." isn't affected.
#
s/\("[^"]+,\) \("\)/\1\2/g

