/** Header file generated with fdesign on Mon Jun 12 06:32:31 2000.**/

#ifndef FD_citation_form_h_
#define FD_citation_form_h_

/** Callbacks, globals and object handlers **/


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *citation_form;
	void *vdata;
	char *cdata;
	long  ldata;
} FD_citation_form;

extern FD_citation_form * create_form_citation_form(void);

#endif /* FD_citation_form_h_ */
