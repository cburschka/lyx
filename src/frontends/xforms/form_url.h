/** Header file generated with fdesign on Wed Aug  2 13:56:57 2000.**/

#ifndef FD_form_url_h_
#define FD_form_url_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormCommandCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormCommandOKCB(FL_OBJECT *, long);


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
