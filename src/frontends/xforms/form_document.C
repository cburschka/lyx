// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 465, 450);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 465, 450, "");
  fdui->tabbed_folder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 20, 15, 435, 365, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 410, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, INPUT);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 245, 410, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, INPUT);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 135, 410, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, INPUT);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 20, 380, 435, 30, "");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 410, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedRestoreCB, INPUT);
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
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 85, 200, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  {
    char const * const dummy = N_("Special:|#S");
    fdui->choice_paperpackage = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 15, 140, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 205, 200, 130, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 165, 190, 170, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 155, 70, 20, _("Margins"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 195, 160, 20, _("Foot/Head Margins"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 440, 360, 10, 10, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 440, 350, 10, 10, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 85, 190, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 75, 95, 20, _("Orientation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->greoup_radio_orientation = fl_bgn_group();
  {
    char const * const dummy = N_("Portrait|#o");
    fdui->radio_portrait = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 90, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Landscape|#L");
    fdui->radio_landscape = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 120, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();

  {
    char const * const dummy = N_("Papersize:|#P");
    fdui->choice_papersize2 = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 15, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 75, 150, 20, _("Custom Papersize"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Use Geometry Package|#U");
    fdui->push_use_geometry = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 45, 170, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Width:|#W");
    fdui->input_custom_width = obj = fl_add_input(FL_NORMAL_INPUT, 330, 100, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Height:|#H");
    fdui->input_custom_height = obj = fl_add_input(FL_NORMAL_INPUT, 330, 140, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Top:|#T");
    fdui->input_top_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 175, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Bottom:|#B");
    fdui->input_bottom_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 215, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Left:|#e");
    fdui->input_left_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 255, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Right:|#R");
    fdui->input_right_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 295, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Headheight:|#i");
    fdui->input_head_height = obj = fl_add_input(FL_NORMAL_INPUT, 330, 215, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Headsep:|#d");
    fdui->input_head_sep = obj = fl_add_input(FL_NORMAL_INPUT, 330, 255, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Footskip:|#F");
    fdui->input_foot_skip = obj = fl_add_input(FL_NORMAL_INPUT, 330, 295, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 10, 360, 420, 20, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 300, 200, 120, 80, _("Separation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 300, 110, 120, 70, _("Page cols"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 300, 20, 120, 70, _("Sides"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Fonts:|#F");
    fdui->choice_doc_fonts = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 100, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Font Size:|#O");
    fdui->choice_doc_fontsize = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 140, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Class:|#C");
    fdui->choice_doc_class = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 20, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, CHECKCHOICECLASS);
  {
    char const * const dummy = N_("Pagestyle:|#P");
    fdui->choice_doc_pagestyle = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 60, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Spacing|#g");
    fdui->choice_doc_spacing = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 290, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Extra Options:|#X");
    fdui->input_doc_extra = obj = fl_add_input(FL_NORMAL_INPUT, 120, 185, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->input_doc_skip = obj = fl_add_input(FL_NORMAL_INPUT, 220, 225, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Default Skip:|#u");
    fdui->choice_doc_skip = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 225, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);

  fdui->group_doc_sides = fl_bgn_group();
  {
    char const * const dummy = N_("One|#n");
    fdui->radio_doc_sides_one = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 30, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Two|#T");
    fdui->radio_doc_sides_two = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 60, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();


  fdui->group_doc_columns = fl_bgn_group();
  {
    char const * const dummy = N_("One|#e");
    fdui->radio_doc_columns_one = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 120, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Two|#w");
    fdui->radio_doc_columns_two = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 150, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fl_end_group();


  fdui->group_doc_sep = fl_bgn_group();
  {
    char const * const dummy = N_("Indent|#I");
    fdui->radio_doc_indent = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 210, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Skip|#K");
    fdui->radio_doc_skip = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 240, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->input_doc_spacing = obj = fl_add_input(FL_NORMAL_INPUT, 300, 290, 120, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
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
    fdui->radio_single = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 180, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Double|#D");
    fdui->radio_double = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 210, 180, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
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
    fdui->input_float_placement = obj = fl_add_input(FL_NORMAL_INPUT, 155, 60, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->slider_secnumdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 155, 110, 80, 30, _("Section number depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  fdui->slider_tocdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 155, 150, 80, 30, _("Table of contents depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("PS Driver:|#S");
    fdui->choice_postscript_driver = obj = fl_add_choice(FL_NORMAL_CHOICE, 155, 205, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
  {
    char const * const dummy = N_("Use AMS Math|#M");
    fdui->check_use_amsmath = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 250, 200, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, INPUT);
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
  fdui->bmtable_bullet_panel = obj = fl_add_bmtable(1, 90, 105, 265, 180, "");
  fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETBMTABLE);
  fl_set_object_lcol(obj, FL_BLUE);
  fl_set_object_boxtype(obj, FL_UP_BOX);
  fl_set_bmtable_pixmap_file(obj, 6, 6,
			     LibFileSearch("images",
					   "standard",
					   "xpm").c_str());
  fl_set_border_width(-1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 95, 20, 255, 70, "");
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
    fdui->radio_bullet_depth_1 = obj = fl_add_button(FL_RADIO_BUTTON, 105, 35, 55, 40, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH1);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("2|#2");
    fdui->radio_bullet_depth_2 = obj = fl_add_button(FL_RADIO_BUTTON, 165, 35, 55, 40, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH2);
  {
    char const * const dummy = N_("3|#3");
    fdui->radio_bullet_depth_3 = obj = fl_add_button(FL_RADIO_BUTTON, 225, 35, 55, 40, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH3);
  {
    char const * const dummy = N_("4|#4");
    fdui->radio_bullet_depth_4 = obj = fl_add_button(FL_RADIO_BUTTON, 285, 35, 55, 40, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETDEPTH4);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 105, 10, 85, 20, _("Bullet Depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  fdui->radio_bullet_panel = fl_bgn_group();
  {
    char const * const dummy = N_("Standard|#S");
    fdui->radio_bullet_panel_standard = obj = fl_add_button(FL_RADIO_BUTTON, 15, 105, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL1);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Maths|#M");
    fdui->radio_bullet_panel_maths = obj = fl_add_button(FL_RADIO_BUTTON, 15, 135, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL2);
  {
    char const * const dummy = N_("Ding 2|#i");
    fdui->radio_bullet_panel_ding2 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 195, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL3);
  {
    char const * const dummy = N_("Ding 3|#n");
    fdui->radio_bullet_panel_ding3 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 225, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL4);
  {
    char const * const dummy = N_("Ding 4|#g");
    fdui->radio_bullet_panel_ding4 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 255, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL5);
  {
    char const * const dummy = N_("Ding 1|#D");
    fdui->radio_bullet_panel_ding1 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 165, 65, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, BULLETPANEL6);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

