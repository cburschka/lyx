// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_delim_h_
#define FD_form_maths_delim_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_delim {
	~FD_form_maths_delim();

	FL_FORM *form;
	FL_OBJECT *bmtable;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_pix;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_ok;
	FL_OBJECT *lado;
	FL_OBJECT *radio_both;
	FL_OBJECT *radio_right;
	FL_OBJECT *radio_left;
};

#endif /* FD_form_maths_delim_h_ */
