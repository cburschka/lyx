#ifndef FD_citation_form_h_
#define FD_citation_form_h_
/* Header file generated with fdesign. */


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *form;
	void *vdata;
	long ldata;
  	FL_OBJECT *box;
  	FL_OBJECT *citeBrsr;
	FL_OBJECT *bibBrsr;
	FL_OBJECT *infoBrsr;
	FL_OBJECT *textAftr;
	FL_OBJECT *addBtn;
	FL_OBJECT *delBtn;
	FL_OBJECT *upBtn;
	FL_OBJECT *downBtn;
	FL_OBJECT *ok;
	FL_OBJECT *cancel;
} FD_citation_form;


typedef struct {
	FL_FORM *bibitem_form;
	void *vdata;
	long ldata;
	FL_OBJECT *key;
	FL_OBJECT *label;
} FD_bibitem_form;

#endif /* FD_citation_form_h_ */
