/** Header file generated with fdesign on Mon Jun 19 12:55:08 2000.**/

#ifndef FD_form_credits_h_
#define FD_form_credits_h_

/** Callbacks, globals and object handlers **/
extern "C" void CreditsOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_credits;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser_credits;
} FD_form_credits;

extern FD_form_credits * create_form_form_credits(void);

#endif /* FD_form_credits_h_ */
