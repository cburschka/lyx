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

# Strip multiple blank lines, leaving only one
/^$/{
N
/^\n$/D
}

# Rewrite FD_form_xxx as FD_xxx
# This is done both for the structs and for the #define bounding the header
s/\(.*\) FD_form_\(.*\)/\1 FD_\2/

# Forward declare FL_FORM and FL_OBJECT
/#define FD/a\
\
#include "forms_fwd.h"

# Delete the fdesign comments
/Callbacks, globals/d
/generated with fdesign/d
/Forms and Objects/d

# Replace lines such as "extern void func(args);"
# with "extern "C" void func(args);"
# Note that these should not occur because we should be using signals. See the
# README file for further information.
s/\(extern \)\(void \)\(.*\)/\1 "C" \2\3/

# Rename the function create_form_form_xxx(void) as build_xxx(void * parent)
s/extern \(.*\) create_form_form_\(.*\)\([(]void[)]\)/\1 build_\2(void *)/

# Manipulate the structs
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

# For all lines containing FL_FORM...
/FL_FORM/{

# Rewrite "FL_FORM *form_xxx;" as "FL_FORM   * form;
# Note that the spaces before FL_FORM are replaced with a <tab>
s/\(.*\)FL_FORM \(.*\)/	FL_FORM   * form;/

# We know that FL_FORM comes before any FL_OBJECT, so can initialise
# the hold space with it.
h
d

}

# For all lines containing FL_OBJECT and not containing extern...
/FL_OBJECT/{
/extern/!{

# perform a little pretty formatting
s/\(.*\)FL_OBJECT \([*]\)\(.*\)/	FL_OBJECT * \3/

# append to the hold space
H
d

}
}

# The struct is ended by "} FD_xxx;", so now's the time to paste back the
# contents of the hold space.
/} FD_/{
# 1. Insert an empty line.
i\

# 2. Rewrite "} FD_xxx;" as   "struct FD_xxx {" and append a d-tor.
s/} \(.*\);/struct \1 {/p
# When the wrappers are here, delete this line and the p-print command above
/struct/s/struct \(.*\) {/	~\1();/

# 3. Paste the contents of the hold space beneath it
G

# 4. Close the struct
a\
};
}
