// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_citation_h_
#define FD_form_citation_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_citation {
	~FD_form_citation();

	FL_FORM *form;
	FL_OBJECT *box;
	FL_OBJECT *browser_cite;
	FL_OBJECT *browser_bib;
	FL_OBJECT *button_add;
	FL_OBJECT *button_del;
	FL_OBJECT *button_up;
	FL_OBJECT *button_down;
	FL_OBJECT *browser_info;
	FL_OBJECT *choice_style;
	FL_OBJECT *input_before;
	FL_OBJECT *input_after;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
};

#endif /* FD_form_citation_h_ */
