// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_deco_h_
#define FD_form_maths_deco_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_deco {
	~FD_form_maths_deco();

	FL_FORM *form;
	FL_OBJECT *bmtable;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_maths_deco_h_ */
