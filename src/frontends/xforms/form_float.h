// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_float_h_
#define FD_form_float_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_float {
	~FD_form_float();

	FL_FORM *form;
	FL_OBJECT *check_top;
	FL_OBJECT *check_bottom;
	FL_OBJECT *check_page;
	FL_OBJECT *check_here;
	FL_OBJECT *check_here_definitely;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_close;
	FL_OBJECT *check_default;
};

#endif /* FD_form_float_h_ */
