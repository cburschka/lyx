// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_tabular_h_
#define FD_form_tabular_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_tabular {
	~FD_form_tabular();

	FL_FORM *form;
	FL_OBJECT *tabFolder;
	FL_OBJECT *button_close;
	FL_OBJECT *input_tabular_column;
	FL_OBJECT *input_tabular_row;
	FL_OBJECT *text_warning;
};
struct FD_form_tabular_options {
	~FD_form_tabular_options();

	FL_FORM *form;
	FL_OBJECT *button_append_column;
	FL_OBJECT *button_delete_column;
	FL_OBJECT *button_append_row;
	FL_OBJECT *button_delete_row;
	FL_OBJECT *button_set_borders;
	FL_OBJECT *button_unset_borders;
	FL_OBJECT *radio_longtable;
	FL_OBJECT *radio_rotate_tabular;
};
struct FD_form_column_options {
	~FD_form_column_options();

	FL_FORM *form;
	FL_OBJECT *radio_border_top;
	FL_OBJECT *radio_border_bottom;
	FL_OBJECT *radio_border_left;
	FL_OBJECT *radio_border_right;
	FL_OBJECT *radio_align_left;
	FL_OBJECT *radio_align_right;
	FL_OBJECT *radio_align_center;
	FL_OBJECT *radio_valign_top;
	FL_OBJECT *radio_valign_center;
	FL_OBJECT *radio_valign_bottom;
	FL_OBJECT *input_column_width;
	FL_OBJECT *input_special_alignment;
};
struct FD_form_cell_options {
	~FD_form_cell_options();

	FL_FORM *form;
	FL_OBJECT *radio_border_top;
	FL_OBJECT *radio_border_bottom;
	FL_OBJECT *radio_border_left;
	FL_OBJECT *radio_border_right;
	FL_OBJECT *radio_align_left;
	FL_OBJECT *radio_align_right;
	FL_OBJECT *radio_align_center;
	FL_OBJECT *radio_valign_top;
	FL_OBJECT *radio_valign_center;
	FL_OBJECT *radio_valign_bottom;
	FL_OBJECT *input_mcolumn_width;
	FL_OBJECT *input_special_multialign;
	FL_OBJECT *radio_multicolumn;
	FL_OBJECT *radio_useminipage;
	FL_OBJECT *radio_rotate_cell;
};
struct FD_form_longtable_options {
	~FD_form_longtable_options();

	FL_FORM *form;
	FL_OBJECT *radio_lt_firsthead;
	FL_OBJECT *radio_lt_head;
	FL_OBJECT *radio_lt_foot;
	FL_OBJECT *radio_lt_lastfoot;
	FL_OBJECT *radio_lt_newpage;
};

#endif /* FD_form_tabular_h_ */
