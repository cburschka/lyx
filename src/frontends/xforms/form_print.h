// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_print_h_
#define FD_form_print_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_print {
	~FD_form_print();

	FL_FORM *form;
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
	FL_OBJECT *radio_collated;
	FL_OBJECT *input_to_page;
};

#endif /* FD_form_print_h_ */
