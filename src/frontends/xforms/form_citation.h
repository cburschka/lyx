/** Header file generated with fdesign on Thu Jul  6 17:01:34 2000.**/

#ifndef FD_form_citation_h_
#define FD_form_citation_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormCitationInputCB(FL_OBJECT *, long);
extern  "C" void C_FormCitationOKCB(FL_OBJECT *, long);
extern  "C" void C_FormCitationCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_citation;
	void *vdata;
	char *cdata;
	long  ldata;
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
} FD_form_citation;


#endif /* FD_form_citation_h_ */
