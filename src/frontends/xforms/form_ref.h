// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_ref_h_
#define FD_form_ref_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_ref {
	~FD_form_ref();

	FL_FORM *form;
	FL_OBJECT *browser;
	FL_OBJECT *update;
	FL_OBJECT *sort;
	FL_OBJECT *name;
	FL_OBJECT *ref;
	FL_OBJECT *type;
	FL_OBJECT *go;
	FL_OBJECT *ok;
	FL_OBJECT *cancel;
};

#endif /* FD_form_ref_h_ */
