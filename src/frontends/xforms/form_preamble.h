// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_preamble_h_
#define FD_form_preamble_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_preamble {
	~FD_form_preamble();

	FL_FORM *form;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *input_preamble;
};

#endif /* FD_form_preamble_h_ */
