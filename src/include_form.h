#ifndef FD_include_h_
#define FD_include_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void include_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *include;
	void *vdata;
	long ldata;
	FL_OBJECT *browsebt;
	FL_OBJECT *flag1;
	FL_OBJECT *input;
	FL_OBJECT *flag41;
	FL_OBJECT *include_grp;
	FL_OBJECT *flag4;
	FL_OBJECT *flag2;
	FL_OBJECT *flag3;
} FD_include;

extern FD_include * create_form_include(void);

#endif /* FD_include_h_ */
