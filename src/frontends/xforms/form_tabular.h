/** Header file generated with fdesign on Wed Jul 12 15:19:16 2000.**/

#ifndef FD_form_tabular_h_
#define FD_form_tabular_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormTabularCloseCB(FL_OBJECT *, long);

extern  "C" void C_FormTabularInputCB(FL_OBJECT *, long);

extern  "C" void C_FormTabularInputCB(FL_OBJECT *, long);

extern  "C" void C_FormTabularInputCB(FL_OBJECT *, long);

extern  "C" void C_FormTabularInputCB(FL_OBJECT *, long);

extern  "C" void C_FormTabularOKCB(FL_OBJECT *, long);
extern  "C" void C_FormTabularApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormTabularCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_tabular;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *tabFolder;
	FL_OBJECT *button_close;
	FL_OBJECT *input_tabular_column;
	FL_OBJECT *input_tabular_row;
	FL_OBJECT *text_warning;
} FD_form_tabular;

typedef struct {
	FL_FORM *form_tabular_options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *button_append_column;
	FL_OBJECT *button_delete_column;
	FL_OBJECT *button_append_row;
	FL_OBJECT *button_delete_row;
	FL_OBJECT *button_set_borders;
	FL_OBJECT *button_unset_borders;
	FL_OBJECT *radio_longtable;
	FL_OBJECT *radio_rotate_tabular;
} FD_form_tabular_options;

typedef struct {
	FL_FORM *form_column_options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *radio_border_top;
	FL_OBJECT *radio_border_bottom;
	FL_OBJECT *radio_border_left;
	FL_OBJECT *radio_border_right;
	FL_OBJECT *radio_align_left;
	FL_OBJECT *radio_align_right;
	FL_OBJECT *radio_align_center;
	FL_OBJECT *input_column_width;
	FL_OBJECT *radio_valign_top;
	FL_OBJECT *radio_valign_center;
	FL_OBJECT *radio_valign_bottom;
	FL_OBJECT *input_special_alignment;
} FD_form_column_options;

typedef struct {
	FL_FORM *form_cell_options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *radio_multicolumn;
	FL_OBJECT *radio_linebreak_cell;
	FL_OBJECT *radio_rotate_cell;
	FL_OBJECT *input_special_multialign;
	FL_OBJECT *input_mcolumn_width;
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
} FD_form_cell_options;

typedef struct {
	FL_FORM *form_longtable_options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *radio_lt_firsthead;
	FL_OBJECT *radio_lt_head;
	FL_OBJECT *radio_lt_foot;
	FL_OBJECT *radio_lt_lastfoot;
	FL_OBJECT *radio_lt_newpage;
} FD_form_longtable_options;

typedef struct {
	FL_FORM *form_create_tabular;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *slider_columns;
	FL_OBJECT *slider_rows;
} FD_form_create_tabular;


#endif /* FD_form_tabular_h_ */
