// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"
#include "bmtable.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_document.h"
#include "FormDocument.h"
#include "bmtable.h"
#include "support/filetools.h"

FD_form_tabbed_document::~FD_form_tabbed_document()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tabbed_document * FormDocument::build_tabbed_document()
{
  FL_OBJECT *obj;
  FD_form_tabbed_document *fdui = new FD_form_tabbed_document;

  fdui->form = fl_bgn_form(FL_NO_BOX, 465, 500);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 465, 500, "");
  fdui->tabbed_folder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 20, 15, 435, 365, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 460, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, INPUT);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 245, 460, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, INPUT);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 135, 460, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, INPUT);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 15, 385, 435, 30, "");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 460, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedRestoreCB, INPUT);
  {
    char const * const dummy = N_("Save as Document Defaults|#v");
    fdui->button_save_defaults = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 420, 195, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Use Class Defaults|#C");
    fdui->button_reset_defaults = obj = fl_add_button(FL_NORMAL_BUTTON, 260, 420, 195, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_doc_paper::~FD_form_doc_paper()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_doc_paper * FormDocument::build_doc_paper()
{
  FL_OBJECT *obj;
  FD_form_doc_paper *fdui = new FD_form_doc_paper;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 345, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 15, 225, 120, _("Papersize"));
  {
    char const * const dummy = N_("Papersize:|#P");
    fdui->choice_papersize = obj = fl_add_choice(FL_NORMAL_CHOICE, 110, 25, 115, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Width:|#W");
    fdui->input_custom_width = obj = fl_add_input(FL_NORMAL_INPUT, 110, 60, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_custom_width_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 170, 60, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Height:|#H");
    fdui->input_custom_height = obj = fl_add_input(FL_NORMAL_INPUT, 110, 95, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_custom_height_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 170, 95, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 245, 15, 180, 120, _("Orientation"));

  fdui->group_radio_orientation = fl_bgn_group();
  {
    char const * const dummy = N_("Portrait|#r");
    fdui->radio_portrait = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Landscape|#L");
    fdui->radio_landscape = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 75, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();

  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 150, 415, 185, _("Margins"));
  {
    char const * const dummy = N_("Custom sizes|#M");
    fdui->check_use_geometry = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 155, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Special (A4 portrait only):|#S");
    fdui->choice_paperpackage = obj = fl_add_choice(FL_NORMAL_CHOICE, 230, 180, 185, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Top:|#T");
    fdui->input_top_margin = obj = fl_add_input(FL_NORMAL_INPUT, 80, 190, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_top_margin_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 140, 190, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom:|#B");
    fdui->input_bottom_margin = obj = fl_add_input(FL_NORMAL_INPUT, 80, 225, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_bottom_margin_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 140, 225, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Inner:|#I");
    fdui->input_inner_margin = obj = fl_add_input(FL_NORMAL_INPUT, 80, 260, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_inner_margin_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 140, 260, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Outer:|#u");
    fdui->input_outer_margin = obj = fl_add_input(FL_NORMAL_INPUT, 80, 295, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_outer_margin_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 140, 295, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Headheight:|#H");
    fdui->input_head_height = obj = fl_add_input(FL_NORMAL_INPUT, 300, 225, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_head_height_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 360, 225, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Headsep:|#d");
    fdui->input_head_sep = obj = fl_add_input(FL_NORMAL_INPUT, 300, 260, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_head_sep_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 360, 260, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Footskip:|#F");
    fdui->input_foot_skip = obj = fl_add_input(FL_NORMAL_INPUT, 300, 295, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_foot_skip_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 360, 295, 55, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_doc_class::~FD_form_doc_class()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_doc_class * FormDocument::build_doc_class()
{
  FL_OBJECT *obj;
  FD_form_doc_class *fdui = new FD_form_doc_class;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 345, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 245, 400, 85, _("Separation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 300, 110, 120, 75, _("Page cols"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 300, 20, 120, 75, _("Sides"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Fonts:|#F");
    fdui->choice_doc_fonts = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 90, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Font Size:|#O");
    fdui->choice_doc_fontsize = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 125, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Class:|#l");
    fdui->choice_doc_class = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 20, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, CHECKCHOICECLASS);
  {
    char const * const dummy = N_("Pagestyle:|#P");
    fdui->choice_doc_pagestyle = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 55, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Spacing|#g");
    fdui->choice_doc_spacing = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 200, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Extra Options:|#X");
    fdui->input_doc_extra = obj = fl_add_input(FL_NORMAL_INPUT, 120, 160, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->input_doc_skip = obj = fl_add_input(FL_NORMAL_INPUT, 285, 290, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Default Skip:|#u");
    fdui->choice_doc_skip = obj = fl_add_choice(FL_NORMAL_CHOICE, 285, 255, 125, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);

  fdui->group_doc_sides = fl_bgn_group();
  {
    char const * const dummy = N_("One|#n");
    fdui->radio_doc_sides_one = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 300, 30, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Two|#T");
    fdui->radio_doc_sides_two = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 300, 60, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();


  fdui->group_doc_columns = fl_bgn_group();
  {
    char const * const dummy = N_("One|#e");
    fdui->radio_doc_columns_one = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 300, 120, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Two|#w");
    fdui->radio_doc_columns_two = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 300, 150, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();


  fdui->group_doc_sep = fl_bgn_group();
  {
    char const * const dummy = N_("Indent|#I");
    fdui->radio_doc_indent = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 260, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Skip|#K");
    fdui->radio_doc_skip = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 290, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->input_doc_spacing = obj = fl_add_input(FL_NORMAL_INPUT, 300, 200, 120, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->choice_default_skip_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 350, 290, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_doc_language::~FD_form_doc_language()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_doc_language * FormDocument::build_doc_language()
{
  FL_OBJECT *obj;
  FD_form_doc_language *fdui = new FD_form_doc_language;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 345, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 40, 120, 290, 110, _("Quote Style    "));
  {
    char const * const dummy = N_("Encoding:|#D");
    fdui->choice_inputenc = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 70, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Type:|#T");
    fdui->choice_quotes_language = obj = fl_add_choice(FL_NORMAL_CHOICE, 110, 140, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
 fl_bgn_group();
  {
    char const * const dummy = N_("Single|#S");
    fdui->radio_single = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 110, 180, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Double|#D");
    fdui->radio_double = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 210, 180, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();

  {
    char const * const dummy = N_("Language:|#L");
    fdui->choice_language = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 35, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_doc_options::~FD_form_doc_options()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_doc_options * FormDocument::build_doc_options()
{
  FL_OBJECT *obj;
  FD_form_doc_options *fdui = new FD_form_doc_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 345, "");
  {
    char const * const dummy = N_("Float Placement:|#L");
    fdui->input_float_placement = obj = fl_add_input(FL_NORMAL_INPUT, 200, 35, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->slider_secnumdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 200, 75, 80, 30, _("Section number depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->slider_tocdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 200, 115, 80, 30, _("Table of contents depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("PS Driver|#S");
    fdui->choice_postscript_driver = obj = fl_add_choice(FL_NORMAL_CHOICE, 200, 155, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Use AMS Math|#M");
    fdui->check_use_amsmath = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 200, 200, 35, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Use Natbib|#N");
    fdui->check_use_natbib = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 200, 245, 35, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Citation style|#i");
    fdui->choice_citation_format = obj = fl_add_choice(FL_NORMAL_CHOICE, 200, 285, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_doc_bullet::~FD_form_doc_bullet()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_doc_bullet * FormDocument::build_doc_bullet()
{
  FL_OBJECT *obj;
  FD_form_doc_bullet *fdui = new FD_form_doc_bullet;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 345);
  fdui->form->u_vdata = this;
  fl_set_border_width(-1);
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 345, "");
  fl_set_border_width(-3);
  fdui->bmtable_bullet_panel = obj = fl_add_bmtable(1, 135, 105, 260, 180, "");
  fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETBMTABLE);
  fl_set_object_lcol(obj, FL_BLUE);
  fl_set_object_boxtype(obj, FL_UP_BOX);
  fl_set_bmtable_pixmap_file(obj, 6, 6,
			     LibFileSearch("images",
				   "standard",
					   "xpm").c_str());
  fl_set_border_width(-1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 135, 20, 260, 55, "");
  {
    char const * const dummy = N_("Size|#z");
    fdui->choice_bullet_size = obj = fl_add_choice(FL_NORMAL_CHOICE, 15, 45, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, CHOICEBULLETSIZE);
  {
    char const * const dummy = N_("LaTeX|#L");
    fdui->input_bullet_latex = obj = fl_add_input(FL_NORMAL_INPUT, 80, 300, 275, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUTBULLETLATEX);

  fdui->radio_bullet_depth = fl_bgn_group();
  {
    char const * const dummy = N_("1|#1");
    fdui->radio_bullet_depth_1 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 150, 35, 40, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH1);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("2|#2");
    fdui->radio_bullet_depth_2 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 210, 35, 40, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH2);
  {
    char const * const dummy = N_("3|#3");
    fdui->radio_bullet_depth_3 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 270, 35, 40, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH3);
  {
    char const * const dummy = N_("4|#4");
    fdui->radio_bullet_depth_4 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 330, 35, 40, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH4);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 145, 10, 85, 20, _("Bullet Depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  fdui->radio_bullet_panel = fl_bgn_group();
  {
    char const * const dummy = N_("Standard|#S");
    fdui->radio_bullet_panel_standard = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 105, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL1);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Maths|#M");
    fdui->radio_bullet_panel_maths = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 135, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL2);
  {
    char const * const dummy = N_("Ding 2|#i");
    fdui->radio_bullet_panel_ding2 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 195, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL3);
  {
    char const * const dummy = N_("Ding 3|#n");
    fdui->radio_bullet_panel_ding3 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 225, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL4);
  {
    char const * const dummy = N_("Ding 4|#g");
    fdui->radio_bullet_panel_ding4 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 255, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL5);
  {
    char const * const dummy = N_("Ding 1|#D");
    fdui->radio_bullet_panel_ding1 = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 165, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL6);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

