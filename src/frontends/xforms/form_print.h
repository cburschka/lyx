/** Header file generated with fdesign on Fri Jul 28 11:50:11 2000.**/

#ifndef FD_form_print_h_
#define FD_form_print_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormPrintInputCB(FL_OBJECT *, long);
extern  "C" void C_FormPrintOKCB(FL_OBJECT *, long);
extern  "C" void C_FormPrintApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormPrintCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_print {

	FL_FORM *form_print;
	FL_OBJECT *input_printer;
	FL_OBJECT *input_file;
	FL_OBJECT *group_radio_printto;
	FL_OBJECT *radio_printer;
	FL_OBJECT *radio_file;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *group_radio_pages;
	FL_OBJECT *radio_all_pages;
	FL_OBJECT *radio_odd_pages;
	FL_OBJECT *radio_even_pages;
	FL_OBJECT *group_radio_order;
	FL_OBJECT *radio_order_normal;
	FL_OBJECT *radio_order_reverse;
	FL_OBJECT *input_from_page;
	FL_OBJECT *input_count;
	FL_OBJECT *radio_unsorted;
	FL_OBJECT *input_to_page;
};

#endif /* FD_form_print_h_ */
