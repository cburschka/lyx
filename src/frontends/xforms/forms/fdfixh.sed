# file fdfixh.sed
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
#
# Full author contact details are available in file CREDITS
#
# Use so:
# sed -f fdfixh.sed somefile.h > fixedfile.h
#
# It contains the instructions that sed requires to manipulate
# the .h files produced by fdesign into a form usable by LyX

# Pretty formatting; remove trailing whitespace.
s/[ 	]*$//


# Delete blank lines, "extern void" function declarations and fdesign comments.
/^$/d
/^extern void/d
/Forms and Objects/d
/Callbacks, globals/d
/generated with fdesign/d


# Immediately before line "#endif", close namespace lyx::frontend
/#endif/i\
\
} // namespace frontend\
} // namespace lyx\

# Immediately after line "#define FD_xxx_h_" that starts off the header file,
# * #include "fdesign_base.h";
# * open namespace lyx::frontend and;
# * append the contents of file EXTERN_FUNCS.
# This latter is a sorted, unique list of any function declarations.
# The actual name of the file is inserted by the parent shell script.
/#define FD/{
a\
\
#include "fdesign_base.h"\
\
namespace lyx {\
namespace frontend {\

r EXTERN_FUNCS
}


# Rewrite FD_form_xxx as FD_xxx.
# This is done both for the structs and for the #define bounding the header.
s/\(.*\) FD_form_\(.*\)/\1 FD_\2/


# Rename the function create_form_form_xxx(void) as build_xxx(void *).
s/extern \(.*\) create_form_form_\(.*\)[(]void[)]/\1 build_\2(void *)/


# Manipulate the structs:
#
# Rewrite                       as
# typedef struct {              struct FD_xxx : public FD_base {
# 	FL_FORM *form_xxx;              FL_OBJECT * some_obj;
# 	void *vdata;                    ...
# 	char *cdata;            };
# 	long  ldata;
#       FL_OBJECT *some_obj;
#       ...
# } FD_xxx;
#
# This is detailed more closely below

# We retain only those lines in the struct that start "	FL_OBJECT *",
# placing them in the hold space until the end of the struct is reached
# and we can ascertain the struct's name.

# All other lines are deleted:
/^typedef struct/d
/^	/{
/FL_OBJECT/!d
}


# For all lines starting with FL_OBJECT...
/^	FL_OBJECT/{

# 1. Perform a little pretty formatting.
s/\(FL_OBJECT \*\)/\1 /

# 2. Append to the hold space and delete from the pattern space.
H
d
}


# The struct is ended by "} FD_xxx;", so now's the time to paste back the
# contents of the hold space.
/^} FD_.*;/{

# 1. Rewrite "} FD_xxx;" as   "\nstruct FD_xxx : public FD_base {".
s/} \(.*\);/\
struct \1 : public FD_base {/

# 2. The hold space contains the FL_OBJECT lines, preceded by a new line.
#    To get rid of this new line, we exchange the contents of the hold and
#    pattern spaces, remove the new line and then exchange back.
x
s/^\n//
x

# 3. Paste the contents of the hold space beneath the "struct FD_xxx" line.
#    and empty the hold space
G
h
s/.*//
x

# 4. Close the struct and append an empty line.
a\
};\

}
