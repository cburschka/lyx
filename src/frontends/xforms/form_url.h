// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_url_h_
#define FD_form_url_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_url {
	~FD_form_url();

	FL_FORM *form;
	FL_OBJECT *url;
	FL_OBJECT *name;
	FL_OBJECT *radio_html;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_restore;
};

#endif /* FD_form_url_h_ */
