// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_include_h_
#define FD_form_include_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_include {
	~FD_form_include();

	FL_FORM *form;
	FL_OBJECT *browsebt;
	FL_OBJECT *flag1;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *loadbt;
	FL_OBJECT *filename;
	FL_OBJECT *flag41;
	FL_OBJECT *include_grp;
	FL_OBJECT *flag4;
	FL_OBJECT *flag2;
	FL_OBJECT *flag3;
};

#endif /* FD_form_include_h_ */
