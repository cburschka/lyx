/** Header file generated with fdesign **/

#ifndef FD_form_title_h_
#define FD_form_title_h_

/** Callbacks, globals and object handlers **/
extern "C" void FigureOKCB(FL_OBJECT *, long);
extern "C" void FigureApplyCB(FL_OBJECT *, long);
extern "C" void FigureCancelCB(FL_OBJECT *, long);

/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_figure;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *group_radio_fugre;
	FL_OBJECT *radio_postscript;
	FL_OBJECT *radio_inline;
} FD_form_figure;

extern FD_form_figure * create_form_form_figure(void);

#endif /* FD_form_title_h_ */
