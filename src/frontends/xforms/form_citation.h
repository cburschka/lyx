/** Header file generated with fdesign on Wed Aug  2 13:56:56 2000.**/

#ifndef FD_form_citation_h_
#define FD_form_citation_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormCommandInputCB(FL_OBJECT *, long);
extern  "C" void C_FormCommandOKCB(FL_OBJECT *, long);
extern  "C" void C_FormCommandCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_citation {

	FL_FORM *form_citation;
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
	FL_OBJECT *ok;
	FL_OBJECT *cancel;
};

#endif /* FD_form_citation_h_ */
