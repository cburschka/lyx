// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_index_h_
#define FD_form_index_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormCommandCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormCommandOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_index {
	~FD_form_index();

	FL_FORM *form;
	FL_OBJECT *key;
	FL_OBJECT *cancel;
	FL_OBJECT *ok;
};

#endif /* FD_form_index_h_ */
