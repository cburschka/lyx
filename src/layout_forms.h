/** Header file generated with fdesign on Mon Apr 12 19:09:42 1999.**/

#ifndef FD_form_document_h_
#define FD_form_document_h_

/** Callbacks, globals and object handlers **/
extern "C" void ChoiceClassCB(FL_OBJECT *, long);
extern "C" void DocumentCancelCB(FL_OBJECT *, long);
extern "C" void DocumentApplyCB(FL_OBJECT *, long);
extern "C" void DocumentOKCB(FL_OBJECT *, long);
extern "C" void DocumentDefskipCB(FL_OBJECT *, long);
extern "C" void DocumentSpacingCB(FL_OBJECT *, long);
extern "C" void DocumentBulletsCB(FL_OBJECT *, long);

extern "C" void CharacterApplyCB(FL_OBJECT *, long);
extern "C" void CharacterCloseCB(FL_OBJECT *, long);

extern "C" void ParagraphOKCB(FL_OBJECT *, long);
extern "C" void ParagraphApplyCB(FL_OBJECT *, long);
extern "C" void ParagraphCancelCB(FL_OBJECT *, long);
extern "C" void ParagraphVSpaceCB(FL_OBJECT *, long);
extern "C" void ParagraphExtraOpen(FL_OBJECT *, long);

extern "C" void PreambleOKCB(FL_OBJECT *, long);
extern "C" void PreambleApplyCB(FL_OBJECT *, long);
extern "C" void PreambleCancelCB(FL_OBJECT *, long);

extern "C" void QuotesOKCB(FL_OBJECT *, long);
extern "C" void QuotesApplyCB(FL_OBJECT *, long);
extern "C" void QuotesCancelCB(FL_OBJECT *, long);

extern "C" void PaperMarginsCB(FL_OBJECT *, long);
extern "C" void PaperCancelCB(FL_OBJECT *, long);
extern "C" void PaperApplyCB(FL_OBJECT *, long);
extern "C" void PaperOKCB(FL_OBJECT *, long);

extern "C" void TableOptCloseCB(FL_OBJECT *, long);
extern "C" void TableOptionsCB(FL_OBJECT *, long);
extern "C" void SetPWidthCB(FL_OBJECT *, long);

extern "C" void CheckPExtraOptCB(FL_OBJECT *, long);
extern "C" void ParagraphExtraOKCB(FL_OBJECT *, long);
extern "C" void ParagraphExtraApplyCB(FL_OBJECT *, long);
extern "C" void ParagraphExtraCancelCB(FL_OBJECT *, long);

extern "C" void TableOptionsCB(FL_OBJECT *, long);
extern "C" void TableSpeCloseCB(FL_OBJECT *, long);

/* Additional Functions/Methods */

#include "bufferparams.h"

extern bool UpdateLayoutDocument(BufferParams *params = 0);
extern bool UpdateLayoutPreamble();
extern bool UpdateLayoutPaper();
extern bool UpdateLayoutQuotes();
extern bool UpdateLayoutParagraph();
extern bool UpdateParagraphExtra();
extern bool UpdateLayoutTable(int);

extern void MenuLayoutCharacter();
extern void MenuLayoutDocument();
extern void MenuLayoutPreamble();
extern void MenuLayoutPaper();
extern void MenuLayoutQuotes();
extern void MenuLayoutParagraph();
extern void MenuLayoutTable(int);

/**** Forms and Objects ****/
typedef struct {
	FL_FORM *form_document;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *group_radio_separation;
	FL_OBJECT *radio_indent;
	FL_OBJECT *radio_skip;
	FL_OBJECT *choice_class;
	FL_OBJECT *choice_pagestyle;
	FL_OBJECT *choice_fonts;
	FL_OBJECT *choice_fontsize;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_ok;
	FL_OBJECT *input_float_placement;
	FL_OBJECT *choice_postscript_driver;
	FL_OBJECT *choice_inputenc;
	FL_OBJECT *group_radio_sides;
	FL_OBJECT *radio_sides_one;
	FL_OBJECT *radio_sides_two;
	FL_OBJECT *group_radio_columns;
	FL_OBJECT *radio_columns_one;
	FL_OBJECT *radio_columns_two;
	FL_OBJECT *input_extra;
	FL_OBJECT *choice_language;
	FL_OBJECT *input_default_skip;
	FL_OBJECT *choice_default_skip;
	FL_OBJECT *slider_secnumdepth;
	FL_OBJECT *slider_tocdepth;
	FL_OBJECT *choice_spacing;
	FL_OBJECT *button_bullets;
	FL_OBJECT *check_use_amsmath;
	FL_OBJECT *input_spacing;
} FD_form_document;

extern FD_form_document * create_form_form_document(void);
typedef struct {
	FL_FORM *form_character;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *choice_family;
	FL_OBJECT *choice_series;
	FL_OBJECT *choice_shape;
	FL_OBJECT *choice_size;
	FL_OBJECT *choice_bar;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_close;
	FL_OBJECT *choice_color;
	FL_OBJECT *check_toggle_all;
} FD_form_character;

extern FD_form_character * create_form_form_character(void);
typedef struct {
	FL_FORM *form_paragraph;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
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
	FL_OBJECT *button_pextra;
	FL_OBJECT *check_space_above;
	FL_OBJECT *check_space_below;
} FD_form_paragraph;

extern FD_form_paragraph * create_form_form_paragraph(void);
typedef struct {
	FL_FORM *form_preamble;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *input_preamble;
} FD_form_preamble;

extern FD_form_preamble * create_form_form_preamble(void);
typedef struct {
	FL_FORM *form_quotes;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *choice_quotes_language;
	FL_OBJECT *group_radio_quotes_number;
	FL_OBJECT *radio_single;
	FL_OBJECT *radio_double;
} FD_form_quotes;

extern FD_form_quotes * create_form_form_quotes(void);
typedef struct {
	FL_FORM *form_paper;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *choice_paperpackage;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_ok;
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
} FD_form_paper;

extern FD_form_paper * create_form_form_paper(void);
typedef struct {
	FL_FORM *form_table_options;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *text_warning;
	FL_OBJECT *button_close;
	FL_OBJECT *radio_border_top;
	FL_OBJECT *radio_border_bottom;
	FL_OBJECT *radio_border_left;
	FL_OBJECT *radio_border_right;
	FL_OBJECT *radio_multicolumn;
	FL_OBJECT *button_append_column;
	FL_OBJECT *button_delete_column;
	FL_OBJECT *button_append_row;
	FL_OBJECT *button_delete_row;
	FL_OBJECT *button_table_delete;
	FL_OBJECT *input_table_column;
	FL_OBJECT *input_table_row;
	FL_OBJECT *button_set_borders;
	FL_OBJECT *button_unset_borders;
	FL_OBJECT *radio_longtable;
	FL_OBJECT *input_column_width;
	FL_OBJECT *radio_rotate_table;
	FL_OBJECT *radio_linebreak_cell;
	FL_OBJECT *radio_lt_firsthead;
	FL_OBJECT *radio_lt_head;
	FL_OBJECT *radio_lt_foot;
	FL_OBJECT *radio_lt_lastfoot;
	FL_OBJECT *radio_lt_newpage;
	FL_OBJECT *radio_rotate_cell;
	FL_OBJECT *button_table_extra;
	FL_OBJECT *radio_align_left;
	FL_OBJECT *radio_align_right;
	FL_OBJECT *radio_align_center;
} FD_form_table_options;

extern FD_form_table_options * create_form_form_table_options(void);
typedef struct {
	FL_FORM *form_paragraph_extra;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *input_pextra_width;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *input_pextra_widthp;
	FL_OBJECT *group_alignment;
	FL_OBJECT *radio_pextra_top;
	FL_OBJECT *radio_pextra_middle;
	FL_OBJECT *radio_pextra_bottom;
	FL_OBJECT *text_warning;
	FL_OBJECT *radio_pextra_hfill;
	FL_OBJECT *radio_pextra_startmp;
	FL_OBJECT *group_extraopt;
	FL_OBJECT *radio_pextra_indent;
	FL_OBJECT *radio_pextra_minipage;
	FL_OBJECT *radio_pextra_floatflt;
} FD_form_paragraph_extra;

extern FD_form_paragraph_extra * create_form_form_paragraph_extra(void);
typedef struct {
	FL_FORM *form_table_extra;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *input_special_alignment;
	FL_OBJECT *input_special_multialign;
} FD_form_table_extra;

extern FD_form_table_extra * create_form_form_table_extra(void);

#endif /* FD_form_document_h_ */
