// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_matrix_h_
#define FD_form_maths_matrix_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_matrix {
	~FD_form_maths_matrix();

	FL_FORM *form;
	FL_OBJECT *slider_rows;
	FL_OBJECT *slider_columns;
	FL_OBJECT *choice_valign;
	FL_OBJECT *input_halign;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_maths_matrix_h_ */
