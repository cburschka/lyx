/** Header file generated with fdesign on Mon Apr 12 19:09:42 1999.**/

#ifndef FD_form_title_h_
#define FD_form_title_h_

/** Callbacks, globals and object handlers **/
extern void TimerCB(FL_OBJECT *, long);

extern void FigureOKCB(FL_OBJECT *, long);
extern void FigureApplyCB(FL_OBJECT *, long);
extern void FigureCancelCB(FL_OBJECT *, long);

extern void ScreenOKCB(FL_OBJECT *, long);
extern void ScreenApplyCB(FL_OBJECT *, long);
extern void ScreenCancelCB(FL_OBJECT *, long);

extern void TocSelectCB(FL_OBJECT *, long);
extern void TocCancelCB(FL_OBJECT *, long);
extern void TocUpdateCB(FL_OBJECT *, long);

extern void RefUpdateCB(FL_OBJECT *, long);
extern void RefHideCB(FL_OBJECT *, long);
extern void RefSelectCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_title;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *timer_title;
} FD_form_title;

extern FD_form_title * create_form_form_title(void);
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
typedef struct {
	FL_FORM *form_screen;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *input_roman;
	FL_OBJECT *input_sans;
	FL_OBJECT *input_typewriter;
	FL_OBJECT *input_font_norm;
	FL_OBJECT *intinput_size;
} FD_form_screen;

extern FD_form_screen * create_form_form_screen(void);
typedef struct {
	FL_FORM *form_toc;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser_toc;
} FD_form_toc;

extern FD_form_toc * create_form_form_toc(void);
typedef struct {
	FL_FORM *form_ref;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser_ref;
	FL_OBJECT *ref_name;
} FD_form_ref;

extern FD_form_ref * create_form_form_ref(void);

#endif /* FD_form_title_h_ */
