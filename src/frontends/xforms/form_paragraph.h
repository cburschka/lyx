// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_paragraph_h_
#define FD_form_paragraph_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_paragraph {
	~FD_form_paragraph();

	FL_FORM *form;
	FL_OBJECT *group_radio_alignment;
	FL_OBJECT *radio_align_right;
	FL_OBJECT *radio_align_left;
	FL_OBJECT *radio_align_block;
	FL_OBJECT *radio_align_center;
	FL_OBJECT *check_lines_top;
	FL_OBJECT *check_lines_bottom;
	FL_OBJECT *check_pagebreaks_top;
	FL_OBJECT *check_pagebreaks_bottom;
	FL_OBJECT *choice_space_above;
	FL_OBJECT *input_space_above;
	FL_OBJECT *check_space_above;
	FL_OBJECT *choice_space_below;
	FL_OBJECT *input_space_below;
	FL_OBJECT *check_space_below;
	FL_OBJECT *choice_linespacing;
	FL_OBJECT *input_linespacing;
	FL_OBJECT *input_labelwidth;
	FL_OBJECT *check_noindent;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *text_warning;
	FL_OBJECT *choice_value_space_above;
	FL_OBJECT *choice_value_space_below;
};

#endif /* FD_form_paragraph_h_ */
