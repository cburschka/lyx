// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_toc_h_
#define FD_form_toc_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_toc {
	~FD_form_toc();

	FL_FORM *form;
	FL_OBJECT *browser_toc;
	FL_OBJECT *button_update;
	FL_OBJECT *choice_toc_type;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_toc_h_ */
