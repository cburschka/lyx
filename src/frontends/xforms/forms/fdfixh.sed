# sed -f fdfixh.sed < somefile.h > fixedfile.h
#
# It contains the instructions that sed requires to manipulate
# the header files produced by fdesign into a form usable by LyX

s/\(extern \)\(.*\)/\1 "C" \2/
#
#  Replaces lines such as "extern void func(args);"
#  with "extern "C" void func(args);"

/create_form_/d
#
#  Deletes lines containing "create_form_"

s,\(generated with fdesign\)\(.*\)\(\*\*/\),\1 \3,
#
# Deletes the dated signature so we can rebuild everything as often
# as we want without silly clashes due to the date.

# Rewrites                 	as
# typedef struct {         	struct FD_form_xxx {
# 	FL_FORM *form_xxx;		~FD_form_xxx();
# 	void *vdata;
# 	char *cdata;			FL_FORM *form;
# 	long  ldata;			...
#        ...			}
# } FD_form_xxx;
#
# This is detailed more closely below 

#Manipulate the struct
#=====================
/typedef struct {/,/} .*;/{

# delete lines containing ...
	/typedef struct {/d
	/vdata/d
	/cdata/d
	/ldata/d

# rewrite "FL_FORM *form_xxx;" as "FL_FORM *form;"
	s/FL_FORM \(.*\);/FL_FORM *form;/

# copy remaining lines into the hold space
	H

# rewrite "} someStruct;" as   "struct someStruct {"    and print
#                              "     ~someStruct();"
	/} \(.*\);/{
		s/} \(.*\);/struct \1 {/p
		s/struct \(.*\) {/	~\1();/p
	}

# delete line from pattern space
	d
}
# swap the hold space back into the pattern space and...
/^$/x

# rewrite "} someStruct;" as "};"
s/} \(.*\);/};/
