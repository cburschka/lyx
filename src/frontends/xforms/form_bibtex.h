// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_bibtex_h_
#define FD_form_bibtex_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_bibtex {
	~FD_form_bibtex();

	FL_FORM *form;
	FL_OBJECT *input_database;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_close;
	FL_OBJECT *input_style;
	FL_OBJECT *button_database_browse;
	FL_OBJECT *button_style_browse;
	FL_OBJECT *check_bibtotoc;
};

#endif /* FD_form_bibtex_h_ */
