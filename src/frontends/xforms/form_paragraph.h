// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_paragraph_general_h_
#define FD_form_paragraph_general_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedRestoreCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_paragraph_general {
	~FD_form_paragraph_general();

	FL_FORM *form;
	FL_OBJECT *input_labelwidth;
	FL_OBJECT *check_lines_top;
	FL_OBJECT *check_lines_bottom;
	FL_OBJECT *check_pagebreaks_top;
	FL_OBJECT *check_pagebreaks_bottom;
	FL_OBJECT *check_noindent;
	FL_OBJECT *group_radio_alignment;
	FL_OBJECT *radio_align_right;
	FL_OBJECT *radio_align_left;
	FL_OBJECT *radio_align_block;
	FL_OBJECT *radio_align_center;
	FL_OBJECT *input_space_above;
	FL_OBJECT *input_space_below;
	FL_OBJECT *choice_space_above;
	FL_OBJECT *choice_space_below;
	FL_OBJECT *check_space_above;
	FL_OBJECT *check_space_below;
};
struct FD_form_paragraph_extra {
	~FD_form_paragraph_extra();

	FL_FORM *form;
	FL_OBJECT *input_pextra_width;
	FL_OBJECT *input_pextra_widthp;
	FL_OBJECT *group_alignment;
	FL_OBJECT *radio_pextra_top;
	FL_OBJECT *radio_pextra_middle;
	FL_OBJECT *radio_pextra_bottom;
	FL_OBJECT *radio_pextra_hfill;
	FL_OBJECT *radio_pextra_startmp;
	FL_OBJECT *group_extraopt;
	FL_OBJECT *radio_pextra_indent;
	FL_OBJECT *radio_pextra_minipage;
	FL_OBJECT *radio_pextra_floatflt;
};
struct FD_form_tabbed_paragraph {
	~FD_form_tabbed_paragraph();

	FL_FORM *form;
	FL_OBJECT *tabbed_folder;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_restore;
	FL_OBJECT *text_warning;
};

#endif /* FD_form_paragraph_general_h_ */
