/** Header file generated with fdesign on Mon Apr 12 19:09:43 1999.**/

#ifndef FD_form_print_h_
#define FD_form_print_h_

/** Callbacks, globals and object handlers **/
extern void PrintOKCB(FL_OBJECT *, long);
extern void PrintApplyCB(FL_OBJECT *, long);
extern void PrintCancelCB(FL_OBJECT *, long);

extern void SendtoOKCB(FL_OBJECT *, long);
extern void SendtoApplyCB(FL_OBJECT *, long);
extern void SendtoCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_print;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *input_printer;
	FL_OBJECT *input_file;
	FL_OBJECT *group_radio_printto;
	FL_OBJECT *radio_printer;
	FL_OBJECT *radio_file;
	FL_OBJECT *group_radio_pages;
	FL_OBJECT *radio_all_pages;
	FL_OBJECT *radio_odd_pages;
	FL_OBJECT *radio_even_pages;
	FL_OBJECT *group_radio_order;
	FL_OBJECT *radio_order_normal;
	FL_OBJECT *radio_order_reverse;
	FL_OBJECT *input_pages;
	FL_OBJECT *input_copies;
	FL_OBJECT *do_unsorted;
} FD_form_print;

extern FD_form_print * create_form_form_print(void);
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

#endif /* FD_form_print_h_ */
