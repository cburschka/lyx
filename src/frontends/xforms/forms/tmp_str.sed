# file tmp_str.sed
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author Angus Leeming
#
# Full author contact details are available in file CREDITS

# This sed script is run on the .C file after the main fdfixc.sed has done
# its stuff. It ensures that any tmp_str variables inserted by fdfixc.sed
# are declared at the top of the appropriate function.
#
# We use a two-pass algorithm like this because a single pass results in
# convoluted sed.


# Initialise the hold space at the start of the function.
/ \* build_/ {
h
d
}


# For all lines within the function...
/^{$/,/^}$/ {

# If it isn't the last line, append it to the hold space.
/^}$/!{
H
d
}

# If it is the last line, paste the contents of the hold space above it,
# seach for the string "tmp_str" and, if found, add its declaration to the top
# of the function.
/^}$/ {
x
G

/tmp_str/s/\(	FL_OBJECT \*\)/	string tmp_str;\
\1/
}

}
