/** Header file generated with fdesign on Mon Jul 31 11:17:12 2000.**/

#ifndef FD_form_title_h_
#define FD_form_title_h_

/** Callbacks, globals and object handlers **/
extern "C" void TimerCB(FL_OBJECT *, long);

extern "C" void FigureOKCB(FL_OBJECT *, long);
extern "C" void FigureApplyCB(FL_OBJECT *, long);
extern "C" void FigureCancelCB(FL_OBJECT *, long);

extern "C" void ScreenOKCB(FL_OBJECT *, long);
extern "C" void ScreenApplyCB(FL_OBJECT *, long);
extern "C" void ScreenCancelCB(FL_OBJECT *, long);

extern "C" void ExternalTemplateCB(FL_OBJECT *, long);
extern "C" void ExternalBrowseCB(FL_OBJECT *, long);
extern "C" void ExternalEditCB(FL_OBJECT *, long);
extern "C" void ExternalViewCB(FL_OBJECT *, long);
extern "C" void ExternalUpdateCB(FL_OBJECT *, long);
extern "C" void ExternalOKCB(FL_OBJECT *, long);
extern "C" void ExternalCancelCB(FL_OBJECT *, long);


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
	FL_FORM *form_external;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *templatechoice;
	FL_OBJECT *filename;
	FL_OBJECT *filenamebrowse;
	FL_OBJECT *parameters;
	FL_OBJECT *edit;
	FL_OBJECT *view;
	FL_OBJECT *update;
	FL_OBJECT *ok;
	FL_OBJECT *cancel;
	FL_OBJECT *helptext;
} FD_form_external;

extern FD_form_external * create_form_form_external(void);

#endif /* FD_form_title_h_ */
