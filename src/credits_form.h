/** Header file generated with fdesign on Mon Apr 12 19:09:41 1999.**/

#ifndef FD_form_credits_h_
#define FD_form_credits_h_

/** Callbacks, globals and object handlers **/
extern void CreditsOKCB(FL_OBJECT *, long);

extern void CopyrightOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_credits;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *browser_credits;
} FD_form_credits;

extern FD_form_credits * create_form_form_credits(void);
typedef struct {
	FL_FORM *copyright;
	void *vdata;
	char *cdata;
	long  ldata;
} FD_copyright;

extern FD_copyright * create_form_copyright(void);

#endif /* FD_form_credits_h_ */
