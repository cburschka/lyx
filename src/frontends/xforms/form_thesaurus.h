// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_thesaurus_h_
#define FD_form_thesaurus_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_thesaurus {
	~FD_form_thesaurus();

	FL_FORM *form;
	FL_OBJECT *button_replace;
	FL_OBJECT *button_close;
	FL_OBJECT *input_entry;
	FL_OBJECT *input_replace;
	FL_OBJECT *browser_meanings;
};

#endif /* FD_form_thesaurus_h_ */
