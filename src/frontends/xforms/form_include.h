// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_include_h_
#define FD_form_include_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_include {
	~FD_form_include();

	FL_FORM *form;
	FL_OBJECT *button_browse;
	FL_OBJECT *check_typeset;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_close;
	FL_OBJECT *button_load;
	FL_OBJECT *input_filename;
	FL_OBJECT *check_visiblespace;
	FL_OBJECT *radio_verbatim;
	FL_OBJECT *radio_useinput;
	FL_OBJECT *radio_useinclude;
};

#endif /* FD_form_include_h_ */
