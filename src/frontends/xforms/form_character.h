// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_character_h_
#define FD_form_character_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_character {
	~FD_form_character();

	FL_FORM *form;
	FL_OBJECT *choice_family;
	FL_OBJECT *choice_series;
	FL_OBJECT *choice_shape;
	FL_OBJECT *choice_size;
	FL_OBJECT *choice_bar;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_close;
	FL_OBJECT *choice_color;
	FL_OBJECT *check_toggle_all;
	FL_OBJECT *choice_language;
};

#endif /* FD_form_character_h_ */
