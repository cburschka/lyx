// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "layout_forms.h"

FD_form_document *create_form_form_document(void)
{
  FL_OBJECT *obj;
  FD_form_document *fdui = (FD_form_document *) fl_calloc(1, sizeof(FD_form_document));

  fdui->form_document = fl_bgn_form(FL_NO_BOX, 580, 430);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 580, 430, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 290, 110, 120, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT, 300, 100, 100, 20, _("Separation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);

  fdui->group_radio_separation = fl_bgn_group();
  fdui->radio_indent = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 290, 120, 110, 30, idex(_("Indent|#I")));fl_set_button_shortcut(obj, scex(_("Indent|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->radio_skip = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 290, 150, 110, 30, idex(_("Skip|#K")));fl_set_button_shortcut(obj, scex(_("Skip|#K")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fl_end_group();

  fdui->choice_class = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 20, 160, 30, idex(_("Class:|#C")));fl_set_button_shortcut(obj, scex(_("Class:|#C")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, ChoiceClassCB, 0);
  fdui->choice_pagestyle = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 60, 160, 30, idex(_("Pagestyle:|#P")));fl_set_button_shortcut(obj, scex(_("Pagestyle:|#P")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->choice_fonts = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 110, 160, 30, idex(_("Fonts:|#F")));fl_set_button_shortcut(obj, scex(_("Fonts:|#F")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->choice_fontsize = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 150, 160, 30, idex(_("Font Size:|#O")));fl_set_button_shortcut(obj, scex(_("Font Size:|#O")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 470, 390, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, DocumentCancelCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 360, 390, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, DocumentApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 250, 390, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, DocumentOKCB, 0);
  fdui->input_float_placement = obj = fl_add_input(FL_NORMAL_INPUT, 450, 195, 120, 30, idex(_("Float Placement:|#L")));fl_set_button_shortcut(obj, scex(_("Float Placement:|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->choice_postscript_driver = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 230, 190, 30, idex(_("PS Driver:|#S")));fl_set_button_shortcut(obj, scex(_("PS Driver:|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->choice_inputenc = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 270, 190, 30, idex(_("Encoding:|#D")));fl_set_button_shortcut(obj, scex(_("Encoding:|#D")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 290, 20, 122, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 432, 20, 136, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);

  fdui->group_radio_sides = fl_bgn_group();
  fdui->radio_sides_one = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 290, 30, 120, 30, idex(_("One|#n")));fl_set_button_shortcut(obj, scex(_("One|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->radio_sides_two = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 290, 60, 120, 30, idex(_("Two|#T")));fl_set_button_shortcut(obj, scex(_("Two|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 300, 10, 60, 20, _("Sides"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT, 440, 10, 80, 20, _("Columns"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);

  fdui->group_radio_columns = fl_bgn_group();
  fdui->radio_columns_one = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 432, 30, 110, 30, idex(_("One|#e")));fl_set_button_shortcut(obj, scex(_("One|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->radio_columns_two = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 432, 60, 110, 30, idex(_("Two|#w")));fl_set_button_shortcut(obj, scex(_("Two|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fl_end_group();

  fdui->input_extra = obj = fl_add_input(FL_NORMAL_INPUT, 450, 235, 120, 30, idex(_("Extra Options:|#X")));fl_set_button_shortcut(obj, scex(_("Extra Options:|#X")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->choice_language = obj = fl_add_box(FL_FRAME_BOX, 120, 190, 190, 30, _("Language:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->input_default_skip = obj = fl_add_input(FL_NORMAL_INPUT, 220, 310, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, DocumentDefskipCB, 0);
  fdui->choice_default_skip = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 310, 90, 30, idex(_("Default Skip:|#u")));fl_set_button_shortcut(obj, scex(_("Default Skip:|#u")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, DocumentDefskipCB, 0);
  fdui->slider_secnumdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 490, 275, 80, 30, _("Section number depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->slider_tocdepth = obj = fl_add_counter(FL_SIMPLE_COUNTER, 490, 315, 80, 30, _("Table of contents depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->choice_spacing = obj = fl_add_choice(FL_NORMAL_CHOICE, 120, 350, 120, 30, idex(_("Spacing|#g")));fl_set_button_shortcut(obj, scex(_("Spacing|#g")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, DocumentSpacingCB, 0);
  fdui->button_bullets = obj = fl_add_button(FL_NORMAL_BUTTON, 430, 110, 130, 30, idex(_("Bullet Shapes|#B")));fl_set_button_shortcut(obj, scex(_("Bullet Shapes|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, DocumentBulletsCB, 0);
  fdui->check_use_amsmath = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 430, 145, 120, 30, idex(_("Use AMS Math|#M")));fl_set_button_shortcut(obj, scex(_("Use AMS Math|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->input_spacing = obj = fl_add_input(FL_FLOAT_INPUT, 250, 350, 95, 30, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, DocumentSpacingCB, 0);
  fl_end_form();

  //fdui->form_document->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_character *create_form_form_character(void)
{
  FL_OBJECT *obj;
  FD_form_character *fdui = (FD_form_character *) fl_calloc(1, sizeof(FD_form_character));

  fdui->form_character = fl_bgn_form(FL_NO_BOX, 300, 375);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 300, 375, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 280, 280, 45, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 220, 280, 45, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 10, 280, 195, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_family = obj = fl_add_choice(FL_NORMAL_CHOICE, 94, 15, 191, 30, idex(_("Family:|#F")));fl_set_button_shortcut(obj, scex(_("Family:|#F")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_series = obj = fl_add_choice(FL_NORMAL_CHOICE, 94, 55, 191, 30, idex(_("Series:|#S")));fl_set_button_shortcut(obj, scex(_("Series:|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_shape = obj = fl_add_choice(FL_NORMAL_CHOICE, 94, 95, 191, 30, idex(_("Shape:|#H")));fl_set_button_shortcut(obj, scex(_("Shape:|#H")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_size = obj = fl_add_choice(FL_NORMAL_CHOICE, 95, 230, 191, 30, idex(_("Size:|#Z")));fl_set_button_shortcut(obj, scex(_("Size:|#Z")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_bar = obj = fl_add_choice(FL_NORMAL_CHOICE, 95, 290, 190, 30, idex(_("Misc:|#M")));fl_set_button_shortcut(obj, scex(_("Misc:|#M")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 330, 80, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, CharacterApplyCB, 0);
  fdui->button_close = obj = fl_add_button(FL_RETURN_BUTTON, 175, 330, 80, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, CharacterCloseCB, 0);
  fdui->choice_color = obj = fl_add_choice(FL_NORMAL_CHOICE, 95, 135, 191, 30, idex(_("Color:|#C")));fl_set_button_shortcut(obj, scex(_("Color:|#C")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->check_toggle_all = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 175, 260, 25, idex(_("Toggle on all these|#T")));fl_set_button_shortcut(obj, scex(_("Toggle on all these|#T")), 1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 210, 210, 20, _("These are never toggled"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 270, 215, 20, _("These are always toggled"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fl_end_form();

  //fdui->form_character->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_paragraph *create_form_form_paragraph(void)
{
  FL_OBJECT *obj;
  FD_form_paragraph *fdui = (FD_form_paragraph *) fl_calloc(1, sizeof(FD_form_paragraph));

  fdui->form_paragraph = fl_bgn_form(FL_NO_BOX, 490, 290);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 490, 290, "");
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 127, 250, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 244, 250, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 360, 250, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphCancelCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 360, 110, 120, 50, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 110, 340, 90, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 360, 20, 120, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 210, 20, 140, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 190, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  fdui->input_labelwidth = obj = fl_add_input(FL_NORMAL_INPUT, 100, 210, 360, 30, idex(_("Label Width:|#d")));fl_set_button_shortcut(obj, scex(_("Label Width:|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_West, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT, 370, 100, 60, 20, _("Indent"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->check_lines_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 30, 120, 30, idex(_("Above|#b")));fl_set_button_shortcut(obj, scex(_("Above|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_lines_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 60, 120, 30, idex(_("Below|#E")));fl_set_button_shortcut(obj, scex(_("Below|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_pagebreaks_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 360, 30, 120, 30, idex(_("Above|#o")));fl_set_button_shortcut(obj, scex(_("Above|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_pagebreaks_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 360, 60, 120, 30, idex(_("Below|#l")));fl_set_button_shortcut(obj, scex(_("Below|#l")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_noindent = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 360, 120, 120, 30, idex(_("No Indent|#I")));fl_set_button_shortcut(obj, scex(_("No Indent|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  fdui->group_radio_alignment = fl_bgn_group();
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 80, 30, idex(_("Right|#R")));fl_set_button_shortcut(obj, scex(_("Right|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 80, 30, idex(_("Left|#f")));fl_set_button_shortcut(obj, scex(_("Left|#f")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_align_block = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 100, 30, 80, 30, idex(_("Block|#c")));fl_set_button_shortcut(obj, scex(_("Block|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 100, 60, 80, 30, idex(_("Center|#n")));fl_set_button_shortcut(obj, scex(_("Center|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  fdui->input_space_above = obj = fl_add_input(FL_NORMAL_INPUT, 180, 120, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, ParagraphVSpaceCB, 0);
  fdui->input_space_below = obj = fl_add_input(FL_NORMAL_INPUT, 180, 160, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, ParagraphVSpaceCB, 0);
  fdui->choice_space_above = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 120, 100, 30, idex(_("Above:|#v")));fl_set_button_shortcut(obj, scex(_("Above:|#v")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, ParagraphVSpaceCB, 0);
  fdui->choice_space_below = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 160, 100, 30, idex(_("Below:|#w")));fl_set_button_shortcut(obj, scex(_("Below:|#w")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, ParagraphVSpaceCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 370, 10, 100, 20, _("Pagebreaks"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 230, 10, 50, 20, _("Lines"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 80, 20, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 100, 130, 20, _("Vertical Spaces"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->button_pextra = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 250, 100, 30, idex(_("ExtraOpt|#X")));fl_set_button_shortcut(obj, scex(_("ExtraOpt|#X")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphExtraOpen, 0);
  fdui->check_space_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 270, 120, 40, 30, idex(_("Keep|#K")));fl_set_button_shortcut(obj, scex(_("Keep|#K")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_space_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 270, 160, 40, 30, idex(_("Keep|#p")));fl_set_button_shortcut(obj, scex(_("Keep|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  //fdui->form_paragraph->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_preamble *create_form_form_preamble(void)
{
  FL_OBJECT *obj;
  FD_form_preamble *fdui = (FD_form_preamble *) fl_calloc(1, sizeof(FD_form_preamble));

  fdui->form_preamble = fl_bgn_form(FL_NO_BOX, 380, 420);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 380, 420, "");
  fdui->button_ok = obj = fl_add_button(FL_NORMAL_BUTTON, 50, 380, 100, 30, idex(_("OK|#O")));fl_set_button_shortcut(obj, scex(_("OK|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, PreambleOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 160, 380, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, PreambleApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 380, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, PreambleCancelCB, 0);
  fdui->input_preamble = obj = fl_add_input(FL_MULTILINE_INPUT, 10, 10, 360, 360, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fl_end_form();

  //fdui->form_preamble->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_quotes *create_form_form_quotes(void)
{
  FL_OBJECT *obj;
  FD_form_quotes *fdui = (FD_form_quotes *) fl_calloc(1, sizeof(FD_form_quotes));

  fdui->form_quotes = fl_bgn_form(FL_NO_BOX, 340, 130);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 130, "");
  obj = fl_add_button(FL_RETURN_BUTTON, 10, 90, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, QuotesOKCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 120, 90, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, QuotesApplyCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 90, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, QuotesCancelCB, 0);
  fdui->choice_quotes_language = obj = fl_add_choice(FL_NORMAL_CHOICE, 90, 10, 160, 30, idex(_("Type:|#T")));fl_set_button_shortcut(obj, scex(_("Type:|#T")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  fdui->group_radio_quotes_number = fl_bgn_group();
  fdui->radio_single = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 90, 50, 80, 30, idex(_("Single|#S")));fl_set_button_shortcut(obj, scex(_("Single|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_double = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 170, 50, 80, 30, idex(_("Double|#D")));fl_set_button_shortcut(obj, scex(_("Double|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 290, 230, 10, 10, _("Text"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  //fdui->form_quotes->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_paper *create_form_form_paper(void)
{
  FL_OBJECT *obj;
  FD_form_paper *fdui = (FD_form_paper *) fl_calloc(1, sizeof(FD_form_paper));

  fdui->form_paper = fl_bgn_form(FL_NO_BOX, 440, 430);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 440, 430, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 100, 200, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->choice_paperpackage = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 20, 140, 30, idex(_("Special:|#S")));fl_set_button_shortcut(obj, scex(_("Special:|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 220, 200, 130, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 180, 190, 170, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 170, 70, 20, _("Margins"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 210, 160, 20, _("Foot/Head Margins"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 320, 390, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperCancelCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 210, 390, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 100, 390, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperOKCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 440, 360, 10, 10, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 440, 350, 10, 10, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 100, 190, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 90, 95, 20, _("Orientation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->greoup_radio_orientation = fl_bgn_group();
  fdui->radio_portrait = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 105, 120, 30, idex(_("Portrait|#o")));fl_set_button_shortcut(obj, scex(_("Portrait|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_landscape = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 135, 120, 30, idex(_("Landscape|#L")));fl_set_button_shortcut(obj, scex(_("Landscape|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  fdui->choice_papersize2 = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 20, 160, 30, idex(_("Papersize:|#P")));fl_set_button_shortcut(obj, scex(_("Papersize:|#P")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 90, 150, 20, _("Custom Papersize"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->push_use_geometry = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 55, 170, 30, idex(_("Use Geometry Package|#U")));fl_set_button_shortcut(obj, scex(_("Use Geometry Package|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_custom_width = obj = fl_add_input(FL_NORMAL_INPUT, 330, 115, 90, 30, idex(_("Width:|#W")));fl_set_button_shortcut(obj, scex(_("Width:|#W")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_custom_height = obj = fl_add_input(FL_NORMAL_INPUT, 330, 155, 90, 30, idex(_("Height:|#H")));fl_set_button_shortcut(obj, scex(_("Height:|#H")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_top_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 190, 90, 30, idex(_("Top:|#T")));fl_set_button_shortcut(obj, scex(_("Top:|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_bottom_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 230, 90, 30, idex(_("Bottom:|#B")));fl_set_button_shortcut(obj, scex(_("Bottom:|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_left_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 270, 90, 30, idex(_("Left:|#e")));fl_set_button_shortcut(obj, scex(_("Left:|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_right_margin = obj = fl_add_input(FL_NORMAL_INPUT, 100, 310, 90, 30, idex(_("Right:|#R")));fl_set_button_shortcut(obj, scex(_("Right:|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_head_height = obj = fl_add_input(FL_NORMAL_INPUT, 330, 230, 90, 30, idex(_("Headheight:|#i")));fl_set_button_shortcut(obj, scex(_("Headheight:|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_head_sep = obj = fl_add_input(FL_NORMAL_INPUT, 330, 270, 90, 30, idex(_("Headsep:|#d")));fl_set_button_shortcut(obj, scex(_("Headsep:|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->input_foot_skip = obj = fl_add_input(FL_NORMAL_INPUT, 330, 310, 90, 30, idex(_("Footskip:|#F")));fl_set_button_shortcut(obj, scex(_("Footskip:|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, PaperMarginsCB, 0);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 10, 360, 420, 20, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  //fdui->form_paper->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_table_options *create_form_form_table_options(void)
{
  FL_OBJECT *obj;
  FD_form_table_options *fdui = (FD_form_table_options *) fl_calloc(1, sizeof(FD_form_table_options));

  fdui->form_table_options = fl_bgn_form(FL_NO_BOX, 540, 310);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 540, 310, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 300, 120, 110, 60, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 300, 20, 110, 90, "");
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 20, 275, 330, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->button_close = obj = fl_add_button(FL_RETURN_BUTTON, 430, 270, 100, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptCloseCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 20, 20, 145, 105, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 30, 10, 70, 20, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 75, 45, 25, 25, idex(_("Top|#T")));fl_set_button_shortcut(obj, scex(_("Top|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 75, 75, 25, 25, idex(_("Bottom|#B")));fl_set_button_shortcut(obj, scex(_("Bottom|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 55, 60, 25, 25, idex(_("Left|#L")));fl_set_button_shortcut(obj, scex(_("Left|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 95, 60, 25, 25, idex(_("Right|#R")));fl_set_button_shortcut(obj, scex(_("Right|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 175, 20, 105, 105, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 180, 10, 90, 20, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 300, 10, 100, 20, _("Special Cell"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_multicolumn = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 30, 95, 25, idex(_("Multicolumn|#M")));fl_set_button_shortcut(obj, scex(_("Multicolumn|#M")), 1);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_append_column = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 170, 120, 30, idex(_("Append Column|#A")));fl_set_button_shortcut(obj, scex(_("Append Column|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_delete_column = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 170, 120, 30, idex(_("Delete Column|#O")));fl_set_button_shortcut(obj, scex(_("Delete Column|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_append_row = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 200, 120, 30, idex(_("Append Row|#p")));fl_set_button_shortcut(obj, scex(_("Append Row|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_delete_row = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 200, 120, 30, idex(_("Delete Row|#w")));fl_set_button_shortcut(obj, scex(_("Delete Row|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_table_delete = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 240, 250, 30, idex(_("Delete Table|#D")));fl_set_button_shortcut(obj, scex(_("Delete Table|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->input_table_column = obj = fl_add_input(FL_NORMAL_INPUT, 290, 240, 60, 30, _("Column"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->input_table_row = obj = fl_add_input(FL_NORMAL_INPUT, 360, 240, 60, 30, _("Row"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->button_set_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 140, 120, 30, idex(_("Set Borders|#S")));fl_set_button_shortcut(obj, scex(_("Set Borders|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_unset_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 140, 120, 30, idex(_("Unset Borders|#U")));fl_set_button_shortcut(obj, scex(_("Unset Borders|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_longtable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 130, 90, 25, _("Longtable"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->input_column_width = obj = fl_add_input(FL_NORMAL_INPUT, 330, 190, 95, 30, _("Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, SetPWidthCB, 0);
  fdui->radio_rotate_table = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 155, 90, 25, idex(_("Rotate 90°|#9")));fl_set_button_shortcut(obj, scex(_("Rotate 90°|#9")), 1);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_linebreak_cell = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 55, 95, 25, idex(_("Linebreaks|#N")));fl_set_button_shortcut(obj, scex(_("Linebreaks|#N")), 1);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 300, 110, 100, 20, _("Spec. Table"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 420, 20, 100, 160, "");
  obj = fl_add_text(FL_NORMAL_TEXT, 425, 10, 85, 20, _("Longtable"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_lt_firsthead = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 420, 30, 90, 25, _("First Head"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_lt_head = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 420, 60, 90, 25, _("Head"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_lt_foot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 420, 90, 90, 25, _("Foot"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_lt_lastfoot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 420, 120, 90, 25, _("Last Foot"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_lt_newpage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 420, 150, 90, 25, _("New Page"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_rotate_cell = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 80, 95, 25, _("Rotate 90°"));
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->button_table_extra = obj = fl_add_button(FL_NORMAL_BUTTON, 430, 235, 100, 30, idex(_("Extra|#X")));fl_set_button_shortcut(obj, scex(_("Extra|#X")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 175, 30, 80, 25, idex(_("Left|#e")));fl_set_button_shortcut(obj, scex(_("Left|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 175, 55, 80, 25, idex(_("Right|#i")));fl_set_button_shortcut(obj, scex(_("Right|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 175, 80, 80, 25, idex(_("Center|#C")));fl_set_button_shortcut(obj, scex(_("Center|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fl_end_form();

  //fdui->form_table_options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_paragraph_extra *create_form_form_paragraph_extra(void)
{
  FL_OBJECT *obj;
  FD_form_paragraph_extra *fdui = (FD_form_paragraph_extra *) fl_calloc(1, sizeof(FD_form_paragraph_extra));

  fdui->form_paragraph_extra = fl_bgn_form(FL_NO_BOX, 460, 250);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 460, 250, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 170, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 350, 20, 100, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 190, 20, 150, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 120, 20, _("Extra Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_pextra_width = obj = fl_add_input(FL_NORMAL_INPUT, 240, 30, 90, 30, idex(_("Length|#L")));fl_set_button_shortcut(obj, scex(_("Length|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 130, 210, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphExtraOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 240, 210, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphExtraApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 210, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ParagraphExtraCancelCB, 0);
  fdui->input_pextra_widthp = obj = fl_add_input(FL_INT_INPUT, 240, 70, 90, 30, idex(_("or %|#o")));fl_set_button_shortcut(obj, scex(_("or %|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 200, 10, 60, 20, _("Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 350, 10, 90, 20, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->group_alignment = fl_bgn_group();
  fdui->radio_pextra_top = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 360, 30, 80, 30, idex(_("Top|#T")));fl_set_button_shortcut(obj, scex(_("Top|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fdui->radio_pextra_middle = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 360, 60, 80, 30, idex(_("Middle|#d")));fl_set_button_shortcut(obj, scex(_("Middle|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fdui->radio_pextra_bottom = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 360, 90, 80, 30, idex(_("Bottom|#B")));fl_set_button_shortcut(obj, scex(_("Bottom|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fl_end_group();

  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 10, 170, 330, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_pextra_hfill = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 130, 300, 30, idex(_("HFill between Minipage paragraphs|#H")));fl_set_button_shortcut(obj, scex(_("HFill between Minipage paragraphs|#H")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_pextra_startmp = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 160, 260, 30, idex(_("Start new Minipage|#S")));fl_set_button_shortcut(obj, scex(_("Start new Minipage|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  fdui->group_extraopt = fl_bgn_group();
  fdui->radio_pextra_indent = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 30, 160, 30, idex(_("Indented Paragraph|#I")));fl_set_button_shortcut(obj, scex(_("Indented Paragraph|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fdui->radio_pextra_minipage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 60, 160, 30, idex(_("Minipage|#M")));fl_set_button_shortcut(obj, scex(_("Minipage|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fdui->radio_pextra_floatflt = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 90, 160, 30, idex(_("Floatflt|#F")));fl_set_button_shortcut(obj, scex(_("Floatflt|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, CheckPExtraOptCB, 0);
  fl_end_group();

  fl_end_form();

  //fdui->form_paragraph_extra->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_table_extra *create_form_form_table_extra(void)
{
  FL_OBJECT *obj;
  FD_form_table_extra *fdui = (FD_form_table_extra *) fl_calloc(1, sizeof(FD_form_table_extra));

  fdui->form_table_extra = fl_bgn_form(FL_NO_BOX, 330, 200);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 330, 200, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 15, 95, 305, 60, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 85, 205, 20, _("Special Multicolumn Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 15, 20, 305, 60, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_special_alignment = obj = fl_add_input(FL_NORMAL_INPUT, 25, 35, 280, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  obj = fl_add_button(FL_RETURN_BUTTON, 220, 160, 100, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, TableSpeCloseCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 170, 20, _("Special Column Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_special_multialign = obj = fl_add_input(FL_NORMAL_INPUT, 25, 110, 280, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, TableOptionsCB, 0);
  fl_end_form();

  //fdui->form_table_extra->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

