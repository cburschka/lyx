// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_tabular.h"
#include "FormTabular.h" 

FD_form_tabular * FormTabular::build_tabular()
{
  FL_OBJECT *obj;
  FD_form_tabular *fdui = new FD_form_tabular;

  fdui->form_tabular = fl_bgn_form(FL_NO_BOX, 510, 295);
  fdui->form_tabular->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 510, 295, "");
  fdui->tabFolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 0, 0, 505, 250, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 415, 260, 90, 30, idex(_("Close|#C")));
    fl_set_button_shortcut(obj, scex(_("Close|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularCloseCB, 0);
  fdui->input_tabular_column = obj = fl_add_input(FL_NORMAL_INPUT, 65, 260, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->input_tabular_row = obj = fl_add_input(FL_NORMAL_INPUT, 5, 260, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 125, 260, 290, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form_tabular->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_tabular_options * FormTabular::build_tabular_options()
{
  FL_OBJECT *obj;
  FD_form_tabular_options *fdui = new FD_form_tabular_options;

  fdui->form_tabular_options = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form_tabular_options->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 505, 227, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 280, 20, 120, 75, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  fdui->button_append_column = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 40, 120, 30, idex(_("Append Column|#A")));
    fl_set_button_shortcut(obj, scex(_("Append Column|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->button_delete_column = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 40, 120, 30, idex(_("Delete Column|#O")));
    fl_set_button_shortcut(obj, scex(_("Delete Column|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->button_append_row = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 70, 120, 30, idex(_("Append Row|#p")));
    fl_set_button_shortcut(obj, scex(_("Append Row|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->button_delete_row = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 70, 120, 30, idex(_("Delete Row|#w")));
    fl_set_button_shortcut(obj, scex(_("Delete Row|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->button_set_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 10, 120, 30, idex(_("Set Borders|#S")));
    fl_set_button_shortcut(obj, scex(_("Set Borders|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->button_unset_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 10, 120, 30, idex(_("Unset Borders|#U")));
    fl_set_button_shortcut(obj, scex(_("Unset Borders|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_longtable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 30, 90, 25, _("Longtable"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_rotate_tabular = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 55, 90, 25, idex(_("Rotate 90°|#9")));
    fl_set_button_shortcut(obj, scex(_("Rotate 90°|#9")), 1);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 285, 10, 100, 20, _("Spec. Table"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form_tabular_options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_column_options * FormTabular::build_column_options()
{
  FL_OBJECT *obj;
  FD_form_column_options *fdui = new FD_form_column_options;

  fdui->form_column_options = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form_column_options->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 505, 227, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 20, 20, 170, 105, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 25, 10, 70, 20, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 45, 25, 25, idex(_("Top|#t")));
    fl_set_button_shortcut(obj, scex(_("Top|#t")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 75, 25, 25, idex(_("Bottom|#b")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 65, 60, 25, 25, idex(_("Left|#l")));
    fl_set_button_shortcut(obj, scex(_("Left|#l")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 105, 60, 25, 25, idex(_("Right|#r")));
    fl_set_button_shortcut(obj, scex(_("Right|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 200, 20, 130, 105, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 205, 10, 115, 20, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 30, 110, 25, idex(_("Left|#e")));
    fl_set_button_shortcut(obj, scex(_("Left|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 80, 115, 25, idex(_("Right|#i")));
    fl_set_button_shortcut(obj, scex(_("Right|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 55, 115, 25, idex(_("Center|#c")));
    fl_set_button_shortcut(obj, scex(_("Center|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->input_column_width = obj = fl_add_input(FL_NORMAL_INPUT, 375, 150, 95, 30, _("Width of column:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 340, 20, 130, 105, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 345, 10, 115, 20, _("V. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 20, 140, 220, 50, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_special_alignment = obj = fl_add_input(FL_NORMAL_INPUT, 30, 150, 200, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 25, 130, 170, 20, _("Special Column Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_valign_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 30, 125, 25, idex(_("Top|#p")));
    fl_set_button_shortcut(obj, scex(_("Top|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_valign_center = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 55, 125, 25, idex(_("Center|#n")));
    fl_set_button_shortcut(obj, scex(_("Center|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_valign_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 80, 120, 25, idex(_("Bottom|#o")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fl_end_form();

  fdui->form_column_options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_cell_options * FormTabular::build_cell_options()
{
  FL_OBJECT *obj;
  FD_form_cell_options *fdui = new FD_form_cell_options;

  fdui->form_cell_options = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form_cell_options->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 505, 227, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 335, 115, 155, 100, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 115, 180, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 105, 70, 20, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 195, 115, 130, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 200, 105, 115, 20, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 345, 105, 115, 20, _("V. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 15, 150, 90, "");
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 5, 140, 20, _("Special Cell"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_multicolumn = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 25, 145, 25, idex(_("Multicolumn|#M")));
    fl_set_button_shortcut(obj, scex(_("Multicolumn|#M")), 1);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_rotate_cell = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 75, 145, 25, _("Rotate 90°"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 165, 15, 325, 90, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_special_multialign = obj = fl_add_input(FL_NORMAL_INPUT, 175, 30, 310, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 170, 5, 290, 20, _("Special Multicolumn Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_mcolumn_width = obj = fl_add_input(FL_NORMAL_INPUT, 360, 65, 125, 30, _("Width of multi-column:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 75, 140, 25, 25, idex(_("Top|#t")));
    fl_set_button_shortcut(obj, scex(_("Top|#t")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 75, 170, 25, 25, idex(_("Bottom|#b")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 55, 155, 25, 25, idex(_("Left|#l")));
    fl_set_button_shortcut(obj, scex(_("Left|#l")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 95, 155, 25, 25, idex(_("Right|#r")));
    fl_set_button_shortcut(obj, scex(_("Right|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 195, 125, 110, 25, idex(_("Left|#e")));
    fl_set_button_shortcut(obj, scex(_("Left|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 195, 175, 115, 25, idex(_("Right|#i")));
    fl_set_button_shortcut(obj, scex(_("Right|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 195, 150, 115, 25, idex(_("Center|#c")));
    fl_set_button_shortcut(obj, scex(_("Center|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_valign_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 125, 140, 25, idex(_("Top|#p")));
    fl_set_button_shortcut(obj, scex(_("Top|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_valign_center = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 150, 140, 25, idex(_("Center|#n")));
    fl_set_button_shortcut(obj, scex(_("Center|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_valign_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 175, 140, 25, idex(_("Bottom|#o")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_useminipage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 50, 145, 25, idex(_("Use Minipage|#s")));
    fl_set_button_shortcut(obj, scex(_("Use Minipage|#s")), 1);
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fl_end_form();

  fdui->form_cell_options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_longtable_options * FormTabular::build_longtable_options()
{
  FL_OBJECT *obj;
  FD_form_longtable_options *fdui = new FD_form_longtable_options;

  fdui->form_longtable_options = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form_longtable_options->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 505, 227, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 100, 75, "");
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 10, 85, 20, _("Header"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_lt_firsthead = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 30, 90, 25, _("First Head"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_lt_head = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 60, 90, 25, _("Head"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 130, 20, 100, 75, "");
  obj = fl_add_text(FL_NORMAL_TEXT, 135, 10, 85, 20, _("Footer"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_lt_foot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 135, 30, 90, 25, _("Foot"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fdui->radio_lt_lastfoot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 135, 60, 90, 25, _("Last Foot"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 245, 20, 95, 75, "");
  obj = fl_add_text(FL_NORMAL_TEXT, 250, 10, 85, 20, _("Special"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->radio_lt_newpage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 250, 30, 90, 25, _("New Page"));
    fl_set_object_callback(obj, C_FormTabularInputCB, 0);
  fl_end_form();

  fdui->form_longtable_options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_create_tabular * FormTabular::build_create_tabular()
{
  FL_OBJECT *obj;
  FD_form_create_tabular *fdui = new FD_form_create_tabular;

  fdui->form_create_tabular = fl_bgn_form(FL_NO_BOX, 310, 130);
  fdui->form_create_tabular->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 310, 130, "");
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 90, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 90, 90, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 210, 90, 90, 30, idex(_("Cancel|^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormTabularCancelCB, 0);
  fdui->slider_columns = obj = fl_add_valslider(FL_HOR_SLIDER, 80, 50, 220, 30, _("Columns"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->slider_rows = obj = fl_add_valslider(FL_HOR_SLIDER, 80, 10, 220, 30, _("Rows"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fl_end_form();

  fdui->form_create_tabular->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

