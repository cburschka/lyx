// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_space_h_
#define FD_form_maths_space_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_space {
	~FD_form_maths_space();

	FL_FORM *form;
	FL_OBJECT *spaces;
	FL_OBJECT *radio_thin;
	FL_OBJECT *radio_medium;
	FL_OBJECT *radio_thick;
	FL_OBJECT *radio_negative;
	FL_OBJECT *radio_quadratin;
	FL_OBJECT *radio_twoquadratin;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_apply;
};

#endif /* FD_form_maths_space_h_ */
