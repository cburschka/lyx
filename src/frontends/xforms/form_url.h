// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_url_h_
#define FD_form_url_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_url {
	~FD_form_url();

	FL_FORM *form;
	FL_OBJECT *url;
	FL_OBJECT *name;
	FL_OBJECT *radio_html;
	FL_OBJECT *cancel;
	FL_OBJECT *ok;
};

#endif /* FD_form_url_h_ */
