// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_spellchecker_h_
#define FD_form_spellchecker_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_spellchecker {
	~FD_form_spellchecker();

	FL_FORM *form;
	FL_OBJECT *text;
	FL_OBJECT *input;
	FL_OBJECT *browser;
	FL_OBJECT *options;
	FL_OBJECT *start;
	FL_OBJECT *insert;
	FL_OBJECT *ignore;
	FL_OBJECT *accept;
	FL_OBJECT *stop;
	FL_OBJECT *done;
	FL_OBJECT *replace;
	FL_OBJECT *slider;
};

#endif /* FD_form_spellchecker_h_ */
