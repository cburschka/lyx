// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_include_h_
#define FD_form_include_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_include {
	~FD_form_include();

	FL_FORM *form;
	FL_OBJECT *button_browse;
	FL_OBJECT *check_typeset;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_load;
	FL_OBJECT *input_filename;
	FL_OBJECT *check_visiblespace;
	FL_OBJECT *include_grp;
	FL_OBJECT *check_verbatim;
	FL_OBJECT *check_useinput;
	FL_OBJECT *check_useinclude;
};

#endif /* FD_form_include_h_ */
