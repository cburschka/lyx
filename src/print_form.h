/** Header file generated with fdesign on Mon Jun 19 12:55:08 2000.**/

#ifndef FD_form_sendto_h_
#define FD_form_sendto_h_

/** Callbacks, globals and object handlers **/
extern "C" void SendtoOKCB(FL_OBJECT *, long);
extern "C" void SendtoApplyCB(FL_OBJECT *, long);
extern "C" void SendtoCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_sendto;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *input_cmd;
	FL_OBJECT *group_ftype;
	FL_OBJECT *radio_ftype_dvi;
	FL_OBJECT *radio_ftype_ps;
	FL_OBJECT *radio_ftype_latex;
	FL_OBJECT *radio_ftype_lyx;
	FL_OBJECT *radio_ftype_ascii;
} FD_form_sendto;

extern FD_form_sendto * create_form_form_sendto(void);

#endif /* FD_form_sendto_h_ */
