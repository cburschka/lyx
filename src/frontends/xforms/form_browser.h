// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_browser_h_
#define FD_form_browser_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_browser {
	~FD_form_browser();

	FL_FORM *form;
	FL_OBJECT *browser;
	FL_OBJECT *button_close;
	FL_OBJECT *button_update;
};

#endif /* FD_form_browser_h_ */
