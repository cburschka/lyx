// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

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
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 410, 100, 30, idex(_("Cancel|#C^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, INPUT);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 245, 410, 100, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, INPUT);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 135, 410, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, INPUT);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 20, 380, 435, 30, "");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 410, 100, 30, idex(_("Restore|#R")));
    fl_set_button_shortcut(obj, scex(_("Restore|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, INPUT);
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
  fdui->choice_paperpackage = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 15, 140, 30, idex(_("Special:|#S")));
    fl_set_button_shortcut(obj, scex(_("Special:|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
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
  fdui->radio_portrait = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 90, 120, 30, idex(_("Portrait|#o")));
    fl_set_button_shortcut(obj, scex(_("Portrait|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->radio_landscape = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 120, 120, 30, idex(_("Landscape|#L")));
    fl_set_button_shortcut(obj, scex(_("Landscape|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fl_end_group();

  fdui->choice_papersize2 = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 15, 160, 30, idex(_("Papersize:|#P")));
    fl_set_button_shortcut(obj, scex(_("Papersize:|#P")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 75, 150, 20, _("Custom Papersize"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->push_use_geometry = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 45, 170, 30, idex(_("Use Geometry Package|#U")));
    fl_set_button_shortcut(obj, scex(_("Use Geometry Package|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_custom_width = obj = fl_add_input(FL_NORMAL_INPUT, 330, 100, 90, 30, idex(_("Width:|#W")));
    fl_set_button_shortcut(obj, scex(_("Width:|#W")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_custom_height = obj = fl_add_input(FL_NORMAL_INPUT, 330, 140, 90, 30, idex(_("Height:|#H")));
    fl_set_button_shortcut(obj, scex(_("Height:|#H")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_top_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 175, 90, 30, idex(_("Top:|#T")));
    fl_set_button_shortcut(obj, scex(_("Top:|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_bottom_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 215, 90, 30, idex(_("Bottom:|#B")));
    fl_set_button_shortcut(obj, scex(_("Bottom:|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_left_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 255, 90, 30, idex(_("Left:|#e")));
    fl_set_button_shortcut(obj, scex(_("Left:|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_right_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 295, 90, 30, idex(_("Right:|#R")));
    fl_set_button_shortcut(obj, scex(_("Right:|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_head_height = obj = fl_add_input(FL_NORMAL_INPUT, 330, 215, 90, 30, idex(_("Headheight:|#i")));
    fl_set_button_shortcut(obj, scex(_("Headheight:|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_head_sep = obj = fl_add_input(FL_NORMAL_INPUT, 330, 255, 90, 30, idex(_("Headsep:|#d")));
    fl_set_button_shortcut(obj, scex(_("Headsep:|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_foot_skip = obj = fl_add_input(FL_NORMAL_INPUT, 330, 295, 90, 30, idex(_("Footskip:|#F")));
    fl_set_button_shortcut(obj, scex(_("Footskip:|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
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
  fdui->choice_doc_fonts = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 100, 160, 30, idex(_("Fonts:|#F")));
    fl_set_button_shortcut(obj, scex(_("Fonts:|#F")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->choice_doc_fontsize = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 140, 160, 30, idex(_("Font Size:|#O")));
    fl_set_button_shortcut(obj, scex(_("Font Size:|#O")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->choice_doc_class = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 20, 160, 30, idex(_("Class:|#C")));
    fl_set_button_shortcut(obj, scex(_("Class:|#C")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKCHOICECLASS);
  fdui->choice_doc_pagestyle = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 60, 160, 30, idex(_("Pagestyle:|#P")));
    fl_set_button_shortcut(obj, scex(_("Pagestyle:|#P")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->choice_doc_spacing = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 290, 160, 30, idex(_("Spacing|#g")));
    fl_set_button_shortcut(obj, scex(_("Spacing|#g")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_doc_extra = obj = fl_add_input(FL_NORMAL_INPUT, 120, 185, 160, 30, idex(_("Extra Options:|#X")));
    fl_set_button_shortcut(obj, scex(_("Extra Options:|#X")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->input_doc_skip = obj = fl_add_input(FL_NORMAL_INPUT, 220, 225, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->choice_doc_skip = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 225, 90, 30, idex(_("Default Skip:|#u")));
    fl_set_button_shortcut(obj, scex(_("Default Skip:|#u")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);

  fdui->group_doc_sides = fl_bgn_group();
  fdui->radio_doc_sides_one = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 30, 120, 30, idex(_("One|#n")));
    fl_set_button_shortcut(obj, scex(_("One|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->radio_doc_sides_two = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 60, 120, 30, idex(_("Two|#T")));
    fl_set_button_shortcut(obj, scex(_("Two|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fl_end_group();


  fdui->group_doc_columns = fl_bgn_group();
  fdui->radio_doc_columns_one = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 120, 110, 30, idex(_("One|#e")));
    fl_set_button_shortcut(obj, scex(_("One|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->radio_doc_columns_two = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 150, 110, 30, idex(_("Two|#w")));
    fl_set_button_shortcut(obj, scex(_("Two|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fl_end_group();


  fdui->group_doc_sep = fl_bgn_group();
  fdui->radio_doc_indent = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 210, 110, 30, idex(_("Indent|#I")));
    fl_set_button_shortcut(obj, scex(_("Indent|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->radio_doc_skip = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 300, 240, 110, 30, idex(_("Skip|#K")));
    fl_set_button_shortcut(obj, scex(_("Skip|#K")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->input_doc_spacing = obj = fl_add_input(FL_NORMAL_INPUT, 300, 290, 120, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
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
  fdui->choice_inputenc = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 70, 190, 30, idex(_("Encoding:|#D")));
    fl_set_button_shortcut(obj, scex(_("Encoding:|#D")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->choice_quotes_language = obj = fl_add_choice(FL_NORMAL_CHOICE, 110, 140, 190, 30, idex(_("Type:|#T")));
    fl_set_button_shortcut(obj, scex(_("Type:|#T")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
 fl_bgn_group();
  fdui->radio_single = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 180, 90, 30, idex(_("Single|#S")));
    fl_set_button_shortcut(obj, scex(_("Single|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->radio_double = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 210, 180, 90, 30, idex(_("Double|#D")));
    fl_set_button_shortcut(obj, scex(_("Double|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fl_end_group();

  fdui->choice_language = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 35, 190, 30, idex(_("Language:|#L")));
    fl_set_button_shortcut(obj, scex(_("Language:|#L")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
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
  fdui->input_float_placement = obj = fl_add_input(FL_NORMAL_INPUT, 155, 60, 120, 30, idex(_("Float Placement:|#L")));
    fl_set_button_shortcut(obj, scex(_("Float Placement:|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->slider_secnumdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 155, 110, 80, 30, _("Section number depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->slider_tocdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 155, 150, 80, 30, _("Table of contents depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->choice_postscript_driver = obj = fl_add_choice(FL_NORMAL_CHOICE, 155, 205, 190, 30, idex(_("PS Driver:|#S")));
    fl_set_button_shortcut(obj, scex(_("PS Driver:|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
  fdui->check_use_amsmath = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 250, 200, 30, idex(_("Use AMS Math|#M")));
    fl_set_button_shortcut(obj, scex(_("Use AMS Math|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUT);
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
  fl_set_object_callback(obj, C_FormBaseInputCB, BULLETBMTABLE);
  fl_set_object_lcol(obj, FL_BLUE);
  fl_set_object_boxtype(obj, FL_UP_BOX);
  fl_set_bmtable_pixmap_file(obj, 6, 6,
			     LibFileSearch("images",
					   "standard",
					   "xpm").c_str());
  fl_set_border_width(-1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 95, 20, 255, 70, "");
  fdui->choice_bullet_size = obj = fl_add_choice(FL_NORMAL_CHOICE, 15, 45, 65, 30, idex(_("Size|#z")));
    fl_set_button_shortcut(obj, scex(_("Size|#z")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, CHOICEBULLETSIZE);
  fdui->input_bullet_latex = obj = fl_add_input(FL_NORMAL_INPUT, 80, 300, 275, 30, idex(_("LaTeX|#L")));
    fl_set_button_shortcut(obj, scex(_("LaTeX|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_callback(obj, C_FormBaseInputCB, INPUTBULLETLATEX);

  fdui->radio_bullet_depth = fl_bgn_group();
  fdui->radio_bullet_depth_1 = obj = fl_add_button(FL_RADIO_BUTTON, 105, 35, 55, 40, idex(_("1|#1")));
    fl_set_button_shortcut(obj, scex(_("1|#1")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETDEPTH1);
    fl_set_button(obj, 1);
  fdui->radio_bullet_depth_2 = obj = fl_add_button(FL_RADIO_BUTTON, 165, 35, 55, 40, idex(_("2|#2")));
    fl_set_button_shortcut(obj, scex(_("2|#2")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETDEPTH2);
  fdui->radio_bullet_depth_3 = obj = fl_add_button(FL_RADIO_BUTTON, 225, 35, 55, 40, idex(_("3|#3")));
    fl_set_button_shortcut(obj, scex(_("3|#3")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETDEPTH3);
  fdui->radio_bullet_depth_4 = obj = fl_add_button(FL_RADIO_BUTTON, 285, 35, 55, 40, idex(_("4|#4")));
    fl_set_button_shortcut(obj, scex(_("4|#4")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETDEPTH4);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 105, 10, 85, 20, _("Bullet Depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  fdui->radio_bullet_panel = fl_bgn_group();
  fdui->radio_bullet_panel_standard = obj = fl_add_button(FL_RADIO_BUTTON, 15, 105, 65, 30, idex(_("Standard|#S")));
    fl_set_button_shortcut(obj, scex(_("Standard|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETPANEL1);
    fl_set_button(obj, 1);
  fdui->radio_bullet_panel_maths = obj = fl_add_button(FL_RADIO_BUTTON, 15, 135, 65, 30, idex(_("Maths|#M")));
    fl_set_button_shortcut(obj, scex(_("Maths|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETPANEL2);
  fdui->radio_bullet_panel_ding2 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 195, 65, 30, idex(_("Ding 2|#i")));
    fl_set_button_shortcut(obj, scex(_("Ding 2|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETPANEL3);
  fdui->radio_bullet_panel_ding3 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 225, 65, 30, idex(_("Ding 3|#n")));
    fl_set_button_shortcut(obj, scex(_("Ding 3|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETPANEL4);
  fdui->radio_bullet_panel_ding4 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 255, 65, 30, idex(_("Ding 4|#g")));
    fl_set_button_shortcut(obj, scex(_("Ding 4|#g")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETPANEL5);
  fdui->radio_bullet_panel_ding1 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 165, 65, 30, idex(_("Ding 1|#D")));
    fl_set_button_shortcut(obj, scex(_("Ding 1|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, BULLETPANEL6);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

