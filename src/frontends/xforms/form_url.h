/** Header file generated with fdesign on Fri Jul 28 11:50:11 2000.**/

#ifndef FD_form_url_h_
#define FD_form_url_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormUrlCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormUrlOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_url {

	FL_FORM *form_url;
	FL_OBJECT *url;
	FL_OBJECT *name;
	FL_OBJECT *radio_html;
	FL_OBJECT *cancel;
	FL_OBJECT *ok;
};

#endif /* FD_form_url_h_ */
