#ifndef FD_form_figure_h_
#define FD_form_figure_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void FigureOKCB(FL_OBJECT *, long);
extern void FigureApplyCB(FL_OBJECT *, long);
extern void FigureCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/

typedef struct {
	FL_FORM *form_figure;
	FL_OBJECT *group_radio_fugre;
	FL_OBJECT *radio_postscript;
	FL_OBJECT *radio_latex;
	void *vdata;
	long ldata;
} FD_form_figure;

extern FD_form_figure * create_form_form_figure(void);

#endif /* FD_form_figure_h_ */
