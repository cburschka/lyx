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
	FL_OBJECT *database;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *style;
	FL_OBJECT *database_browse;
	FL_OBJECT *style_browse;
	FL_OBJECT *radio_bibtotoc;
	FL_OBJECT *text_info;
};

#endif /* FD_form_bibtex_h_ */
