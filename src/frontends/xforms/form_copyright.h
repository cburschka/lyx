// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_copyright_h_
#define FD_form_copyright_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_copyright {
	~FD_form_copyright();

	FL_FORM *form;
	FL_OBJECT *text_copyright;
	FL_OBJECT *text_licence;
	FL_OBJECT *text_disclaimer;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_copyright_h_ */
