// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_panel_h_
#define FD_form_panel_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormMathsButtonCB(FL_OBJECT *, long);

extern  "C" void C_FormMathsDelimCB(FL_OBJECT *, long);

extern  "C" void C_FormMathsMatrixCB(FL_OBJECT *, long);

extern  "C" void C_FormMathsDecoCB(FL_OBJECT *, long);

extern  "C" void C_FormMathsSpaceCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_panel {
	~FD_form_panel();

	FL_FORM *form;
	FL_OBJECT *button_close;
	FL_OBJECT *browser_functions;
	FL_OBJECT *button_greek;
	FL_OBJECT *button_arrow;
	FL_OBJECT *button_boperator;
	FL_OBJECT *button_brelats;
	FL_OBJECT *button_varsize;
	FL_OBJECT *button_misc;
	FL_OBJECT *button_equation;
	FL_OBJECT *button_sqrt;
	FL_OBJECT *button_frac;
	FL_OBJECT *button_delim;
	FL_OBJECT *button_matrix;
	FL_OBJECT *button_deco;
	FL_OBJECT *button_space;
};
struct FD_form_delim {
	~FD_form_delim();

	FL_FORM *form;
	FL_OBJECT *bmtable_delim;
	FL_OBJECT *lado;
	FL_OBJECT *radio_right;
	FL_OBJECT *radio_left;
	FL_OBJECT *button_delim_close;
	FL_OBJECT *button_delim_pix;
	FL_OBJECT *button_delim_apply;
	FL_OBJECT *button_delim_ok;
};
struct FD_form_matrix {
	~FD_form_matrix();

	FL_FORM *form;
	FL_OBJECT *button_matrix_ok;
	FL_OBJECT *button_matrix_close;
	FL_OBJECT *slider_matrix_rows;
	FL_OBJECT *slider_matrix_columns;
	FL_OBJECT *choice_matrix_valign;
	FL_OBJECT *input_matrix_halign;
	FL_OBJECT *button_matrix_apply;
};
struct FD_form_deco {
	~FD_form_deco();

	FL_FORM *form;
	FL_OBJECT *bmtable_deco;
	FL_OBJECT *button_deco_close;
};
struct FD_form_space {
	~FD_form_space();

	FL_FORM *form;
	FL_OBJECT *button_space_menu;
	FL_OBJECT *button_space_cancel;
	FL_OBJECT *button_space_apply;
	FL_OBJECT *spaces;
	FL_OBJECT *radio_space_thin;
	FL_OBJECT *radio_space_medium;
	FL_OBJECT *radio_space_thick;
	FL_OBJECT *radio_space_negative;
	FL_OBJECT *radio_space_quadratin;
	FL_OBJECT *radio_space_twoquadratin;
};

#endif /* FD_form_panel_h_ */
