// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_ref_h_
#define FD_form_ref_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_ref {
	~FD_form_ref();

	FL_FORM *form;
	FL_OBJECT *choice_buffer;
	FL_OBJECT *browser_refs;
	FL_OBJECT *button_update;
	FL_OBJECT *check_sort;
	FL_OBJECT *input_name;
	FL_OBJECT *input_ref;
	FL_OBJECT *choice_type;
	FL_OBJECT *button_go;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_close;
	FL_OBJECT *button_apply;
};

#endif /* FD_form_ref_h_ */
