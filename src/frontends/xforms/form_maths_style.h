// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_maths_style_h_
#define FD_form_maths_style_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_maths_style {
	~FD_form_maths_style();

	FL_FORM *form;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *styles;
	FL_OBJECT *radio_display;
	FL_OBJECT *radio_text;
	FL_OBJECT *radio_script;
	FL_OBJECT *radio_bold;
	FL_OBJECT *radio_calligraphic;
	FL_OBJECT *radio_roman;
	FL_OBJECT *radio_scriptscript;
	FL_OBJECT *radio_typewriter;
	FL_OBJECT *radio_italic;
	FL_OBJECT *radio_bbbold;
	FL_OBJECT *radio_fraktur;
	FL_OBJECT *radio_normal;
	FL_OBJECT *radio_textrm;
	FL_OBJECT *radio_sans;
};

#endif /* FD_form_maths_style_h_ */
