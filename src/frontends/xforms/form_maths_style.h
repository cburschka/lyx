// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_style_h_
#define FD_form_maths_style_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_style {
	~FD_form_maths_style();

	FL_FORM *form;
	FL_OBJECT *bmtable_font2;
	FL_OBJECT *bmtable_style1;
	FL_OBJECT *button_close;
	FL_OBJECT *bmtable_style2;
	FL_OBJECT *bmtable_font1;
	FL_OBJECT *button_reset;
	FL_OBJECT *button_textrm;
};

#endif /* FD_form_maths_style_h_ */
