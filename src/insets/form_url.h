/** Header file generated with fdesign on Mon Oct 25 15:21:02 1999.**/

#ifndef FD_form_url_h_
#define FD_form_url_h_

/** Callbacks, globals and object handlers **/
extern "C" void C_InsetUrl_CloseUrlCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_url;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *url_name;
	FL_OBJECT *name_name;
	FL_OBJECT *radio_html;
	FL_OBJECT *button_close;
} FD_form_url;

extern FD_form_url * create_form_form_url(void);

#endif /* FD_form_url_h_ */
