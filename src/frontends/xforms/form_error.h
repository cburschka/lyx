// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_error_h_
#define FD_form_error_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_error {
	~FD_form_error();

	FL_FORM *form;
	FL_OBJECT *frame_message;
	FL_OBJECT *button_close;
};

#endif /* FD_form_error_h_ */
