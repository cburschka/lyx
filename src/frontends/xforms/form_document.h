// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_tabbed_document_h_
#define FD_form_tabbed_document_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedRestoreCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_tabbed_document {
	~FD_form_tabbed_document();

	FL_FORM *form;
	FL_OBJECT *tabbed_folder;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_ok;
	FL_OBJECT *text_warning;
	FL_OBJECT *button_restore;
};
struct FD_form_doc_paper {
	~FD_form_doc_paper();

	FL_FORM *form;
	FL_OBJECT *choice_paperpackage;
	FL_OBJECT *greoup_radio_orientation;
	FL_OBJECT *radio_portrait;
	FL_OBJECT *radio_landscape;
	FL_OBJECT *choice_papersize2;
	FL_OBJECT *push_use_geometry;
	FL_OBJECT *input_custom_width;
	FL_OBJECT *input_custom_height;
	FL_OBJECT *input_top_margin;
	FL_OBJECT *input_bottom_margin;
	FL_OBJECT *input_left_margin;
	FL_OBJECT *input_right_margin;
	FL_OBJECT *input_head_height;
	FL_OBJECT *input_head_sep;
	FL_OBJECT *input_foot_skip;
	FL_OBJECT *text_warning;
};
struct FD_form_doc_class {
	~FD_form_doc_class();

	FL_FORM *form;
	FL_OBJECT *choice_doc_fonts;
	FL_OBJECT *choice_doc_fontsize;
	FL_OBJECT *choice_doc_class;
	FL_OBJECT *choice_doc_pagestyle;
	FL_OBJECT *choice_doc_spacing;
	FL_OBJECT *input_doc_extra;
	FL_OBJECT *input_doc_skip;
	FL_OBJECT *choice_doc_skip;
	FL_OBJECT *group_doc_sides;
	FL_OBJECT *radio_doc_sides_one;
	FL_OBJECT *radio_doc_sides_two;
	FL_OBJECT *group_doc_columns;
	FL_OBJECT *radio_doc_columns_one;
	FL_OBJECT *radio_doc_columns_two;
	FL_OBJECT *group_doc_sep;
	FL_OBJECT *radio_doc_indent;
	FL_OBJECT *radio_doc_skip;
	FL_OBJECT *input_doc_spacing;
};
struct FD_form_doc_language {
	~FD_form_doc_language();

	FL_FORM *form;
	FL_OBJECT *choice_inputenc;
	FL_OBJECT *choice_quotes_language;
	FL_OBJECT *radio_single;
	FL_OBJECT *radio_double;
	FL_OBJECT *choice_language;
};
struct FD_form_doc_options {
	~FD_form_doc_options();

	FL_FORM *form;
	FL_OBJECT *input_float_placement;
	FL_OBJECT *slider_secnumdepth;
	FL_OBJECT *slider_tocdepth;
	FL_OBJECT *choice_postscript_driver;
	FL_OBJECT *check_use_amsmath;
	FL_OBJECT *check_use_natbib;
	FL_OBJECT *choice_citation_format;
};
struct FD_form_doc_bullet {
	~FD_form_doc_bullet();

	FL_FORM *form;
	FL_OBJECT *bmtable_bullet_panel;
	FL_OBJECT *choice_bullet_size;
	FL_OBJECT *input_bullet_latex;
	FL_OBJECT *radio_bullet_depth;
	FL_OBJECT *radio_bullet_depth_1;
	FL_OBJECT *radio_bullet_depth_2;
	FL_OBJECT *radio_bullet_depth_3;
	FL_OBJECT *radio_bullet_depth_4;
	FL_OBJECT *radio_bullet_panel;
	FL_OBJECT *radio_bullet_panel_standard;
	FL_OBJECT *radio_bullet_panel_maths;
	FL_OBJECT *radio_bullet_panel_ding2;
	FL_OBJECT *radio_bullet_panel_ding3;
	FL_OBJECT *radio_bullet_panel_ding4;
	FL_OBJECT *radio_bullet_panel_ding1;
};

#endif /* FD_form_tabbed_document_h_ */
