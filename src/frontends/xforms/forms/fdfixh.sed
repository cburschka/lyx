# cat someFile.h | sed -f fdfixh.sed

# It contains the instructions that sed requires to manipulate
# the header files produced by fdesign into a form usable by LyX

# In summary it...

# s/\(extern \)\(.*\)/\1 "C" \2/
#
#  Replaces lines such as "extern void func(args);"
#  with "extern "C" void func(args);"

# /create_form_/d
#
#  Deletes lines containing "/create_form_/d"

# Rewrites                        as
# typedef struct {                struct someStruct {
#        ...                              ...
#        void *vdata;                     ...
#        char *cdata;             };
#        long  ldata;
#        ...
# } someStruct;
#
# This is detailed more closely below 

s/\(extern \)\(.*\)/\1 "C" \2/
/create_form_/d

/typedef struct {/,/} .*;/{  
# delete lines containing ...
/typedef struct {/d
/vdata/d
/cdata/d
/ldata/d

# copy remaining lines into the hold space
H

# rewrite "} someStruct;" as "struct someStruct {" and print
s/} \(.*\);/struct \1 {/p

# delete line from pattern space
d
}
# swap the hold space back into the pattern space and...
/^$/x
{
# rewrite "} someStruct;" as "};"
s/} \(.*\);/};/
}
