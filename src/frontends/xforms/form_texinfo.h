// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_texinfo_h_
#define FD_form_texinfo_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_texinfo {
	~FD_form_texinfo();

	FL_FORM *form;
	FL_OBJECT *browser;
	FL_OBJECT *radio_cls;
	FL_OBJECT *radio_sty;
	FL_OBJECT *radio_bst;
	FL_OBJECT *button_rescan;
	FL_OBJECT *button_view;
	FL_OBJECT *button_fullPath;
	FL_OBJECT *message;
	FL_OBJECT *help;
	FL_OBJECT *button_texhash;
	FL_OBJECT *button_close;
};

#endif /* FD_form_texinfo_h_ */
