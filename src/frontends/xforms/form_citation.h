// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_citation_h_
#define FD_form_citation_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseRestoreCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_citation {
	~FD_form_citation();

	FL_FORM *form;
	FL_OBJECT *box;
	FL_OBJECT *citeBrsr;
	FL_OBJECT *bibBrsr;
	FL_OBJECT *addBtn;
	FL_OBJECT *delBtn;
	FL_OBJECT *upBtn;
	FL_OBJECT *downBtn;
	FL_OBJECT *infoBrsr;
	FL_OBJECT *style;
	FL_OBJECT *textBefore;
	FL_OBJECT *textAftr;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_restore;
};

#endif /* FD_form_citation_h_ */
