# Create the destructor.
/\(.*\) *create_form_form/{
h
s/\(.*\) \*create_form_form[^ ]*/\1::~\1()/p
/\(.*\)::~\(.*\)()/{
i\
{\
  if ( form->visible ) fl_hide_form( form );\
  fl_free_form( form );\
}\
\

}
x
}

# Rewrite "fdui->form_xxx->fdui = ..." as "fdui->form->fdui = ..."
s/fdui->form_\([^ ]*\)->fdui =/fdui->form->fdui =/

# Rewrite "fdui->form_xxx = ..." as "fdui->form = ..."
s/fdui->form_\([^ ]*\) =/fdui->form =/

# Rewrite "fdui->form_xxx->..." as "fdui->form->..."
s/fdui->form_\([^ ]*\)->/fdui->form->/

#  Replace "forms.h" by FORMS_H_LOCATION in #include directives. This
#  macro is defined in config.h and is either <forms.h> or
#  <X11/forms.h>. 
s/#include \"forms\.h\"/#include FORMS_H_LOCATION/

#   For all lines containing "#include "form_*"", append a line
#   containing the header file of the parent class
/#include \"form_.*\"/a\
#include \"CLASSNAME.h\"

#  For all lines containing "fl_" and a string _not_ containing |,
#  replace the string with _(string)
/fl_/ s/".[^|]*"/_(&)/

#  For all lines containing "shortcut" and a string containing |, 
#  replace the string with scex(_(string))
/shortcut/ s/".*[|].*"/scex(_(&))/

#  For all lines containing "fl_add" and a string containing |, replace
#  the string with idex(_(string))
/fl_add/ s/".*[|].*"/idex(_(&))/

#  For all lines containing "fl_add" and a string containing |, add the
#  shortcut command after the end of this line
/fl_add/ s/idex(\(.*\)").*$/&\
    fl_set_button_shortcut(obj,scex(\1")),1);/

#  gettext will get confused if the string contains a "%" unless the line is
#  preceeded immediately by //xgettext:no-c-format
/_(".*[%].*")/i\
  // xgettext:no-c-format

# We use new/delete not malloc/free so change to suit.
s/\(\(FD_[^ ]*\) \*fdui =\).*sizeof(\*fdui))/\1 new \2/

# Fixup the name of the create_form... function to have a signature
# matching that of the method it will become.
s/\(FD_f\([^ _]*\)_\([^ ]*\)\) \*create_form_form[^ ]*/\1 * CLASSNAME::build_\3()/

# We need to store a pointer to the dialog in u_vdata so that the
# callbacks will work.
s/\(fdui->form\)\(.*bgn_form.*\)/\1\2\
  \1->u_vdata = this;/

# Someone got busy and put spaces in after commas but didn't allow for the
# autogeneration of the files so their pretty formatting got lost. Not anymore.
#
s/,\([^ ]\)/, \1/g

# Clean up one special case where a comma appears at the end of a string
# while ensuring "...", "..." isn't affected.
#
s/\("[^"]+,\) \("\)/\1\2/g

