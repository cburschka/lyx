// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_panel_h_
#define FD_form_maths_panel_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_panel {
	~FD_form_maths_panel();

	FL_FORM *form;
	FL_OBJECT *button_close;
	FL_OBJECT *browser_funcs;
	FL_OBJECT *button_greek;
	FL_OBJECT *button_arrow;
	FL_OBJECT *button_boperator;
	FL_OBJECT *button_brelats;
	FL_OBJECT *button_misc;
	FL_OBJECT *button_equation;
	FL_OBJECT *button_sqrt;
	FL_OBJECT *button_frac;
	FL_OBJECT *button_delim;
	FL_OBJECT *button_matrix;
	FL_OBJECT *button_deco;
	FL_OBJECT *button_space;
	FL_OBJECT *button_dots;
	FL_OBJECT *button_varsize;
	FL_OBJECT *button_sub;
	FL_OBJECT *button_super;
	FL_OBJECT *button_style;
};

#endif /* FD_form_maths_panel_h_ */
