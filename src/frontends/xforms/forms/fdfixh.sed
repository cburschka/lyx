# file fdfixh.sed
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
#
# Full author contact details are available in file CREDITS
#
# Use so:
# sed -f fdfixh.sed < somefile.h > fixedfile.h
#
# It contains the instructions that sed requires to manipulate
# the .h files produced by fdesign into a form usable by LyX

# Remove trailing whitespace.
s/[ 	]*$//


# Delete blank lines, "extern void" function declarations and fdesign comments.
/^$/d
/^extern void/d
/Forms and Objects/d
/Callbacks, globals/d
/generated with fdesign/d


# Pretty formatting; add an empty line before "#endif"
/#endif/i\


# Immediately after line "#define FD_xxx_h_" that starts off the header file,
# forward declare FL_FORM and FL_OBJECT and append the contents of file
# "extern.tmp". This latter is a sorted, unique list of any function
# declarations.
/#define FD/{
a\
\
#include "forms_fwd.h"\

r extern.tmp
}


# Rewrite FD_form_xxx as FD_xxx.
# This is done both for the structs and for the #define bounding the header.
s/\(.*\) FD_form_\(.*\)/\1 FD_\2/


# Rename the function create_form_form_xxx(void) as build_xxx(void *).
s/extern \(.*\) create_form_form_\(.*\)[(]void[)]/\1 build_\2(void *)/


# Manipulate the structs:
#
# Rewrite                       as
# typedef struct {              struct FD_xxx {
# 	FL_FORM *form_xxx;              ~FD_xxx();
# 	void *vdata;                    FL_FORM   * form;
# 	char *cdata;                    FL_OBJECT * some_obj;
# 	long  ldata;                    ...
#       FL_OBJECT *some_obj;    };
#       ...
# } FD_xxx;
#
# This is detailed more closely below 

# Delete lines containing:
/typedef struct/d
/vdata/d
/cdata/d
/ldata/d

# Place all lines containing FL_FORM and FL_OBJECT in the hold space, deleting
# them from the pattern space.

# For all lines starting with FL_FORM...
/^	FL_FORM/{

# 1. Rewrite "FL_FORM *form_xxx;" as "FL_FORM   * form;
s/FL_FORM[ ]*\*form.*/FL_FORM   * form;/

# 2. We know that FL_FORM comes before any FL_OBJECT, so can initialise
# the hold space with it. Delete from the pattern space.
h
d
}

# For all lines starting with FL_OBJECT...
/^	FL_OBJECT/{

# 1. Perform a little pretty formatting.
s/FL_OBJECT \*\(.*\)/FL_OBJECT * \1/

# 2. Append to the hold space and delete from the pattern space.
H
d
}

# The struct is ended by "} FD_xxx;", so now's the time to paste back the
# contents of the hold space.
/} FD_/{
# 1. Insert an empty line.
i\

# 2. Rewrite "} FD_xxx;" as   "struct FD_xxx {" and append a d-tor.
s/} \(.*\);/struct \1 {\
	~\1();/

# 3. Paste the contents of the hold space beneath it.
G

# 4. Close the struct and append an empty line.
a\
};\

}
