// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_tabular.h"
#include "FormTabular.h"

FD_form_tabular::~FD_form_tabular()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tabular * FormTabular::build_tabular()
{
  FL_OBJECT *obj;
  FD_form_tabular *fdui = new FD_form_tabular;

  fdui->form = fl_bgn_form(FL_NO_BOX, 510, 295);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 510, 295, "");
  fdui->tabFolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 0, 0, 505, 250, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 415, 260, 90, 30, idex(_("Close|#C^[")));
    fl_set_button_shortcut(obj, scex(_("Close|#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
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

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_tabular_options::~FD_form_tabular_options()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tabular_options * FormTabular::build_tabular_options()
{
  FL_OBJECT *obj;
  FD_form_tabular_options *fdui = new FD_form_tabular_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 225);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 225, "");
  fdui->button_append_column = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 40, 120, 30, idex(_("Append Column|#A")));
    fl_set_button_shortcut(obj, scex(_("Append Column|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_delete_column = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 40, 120, 30, idex(_("Delete Column|#O")));
    fl_set_button_shortcut(obj, scex(_("Delete Column|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_append_row = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 70, 120, 30, idex(_("Append Row|#p")));
    fl_set_button_shortcut(obj, scex(_("Append Row|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_delete_row = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 70, 120, 30, idex(_("Delete Row|#w")));
    fl_set_button_shortcut(obj, scex(_("Delete Row|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_set_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 10, 120, 30, idex(_("Set Borders|#S")));
    fl_set_button_shortcut(obj, scex(_("Set Borders|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_unset_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 10, 120, 30, idex(_("Unset Borders|#U")));
    fl_set_button_shortcut(obj, scex(_("Unset Borders|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_longtable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 30, 90, 25, idex(_("Longtable|#L")));
    fl_set_button_shortcut(obj, scex(_("Longtable|#L")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_rotate_tabular = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 55, 90, 25, idex(_("Rotate 90院#9")));
    fl_set_button_shortcut(obj, scex(_("Rotate 90院#9")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 280, 20, 120, 75, _("Spec. Table"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_column_options::~FD_form_column_options()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_column_options * FormTabular::build_column_options()
{
  FL_OBJECT *obj;
  FD_form_column_options *fdui = new FD_form_column_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 227, "");
  fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 45, 25, 25, idex(_("Top|#t")));
    fl_set_button_shortcut(obj, scex(_("Top|#t")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 75, 25, 25, idex(_("Bottom|#b")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 65, 60, 25, 25, idex(_("Left|#l")));
    fl_set_button_shortcut(obj, scex(_("Left|#l")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 105, 60, 25, 25, idex(_("Right|#r")));
    fl_set_button_shortcut(obj, scex(_("Right|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 30, 110, 25, idex(_("Left|#e")));
    fl_set_button_shortcut(obj, scex(_("Left|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 80, 115, 25, idex(_("Right|#i")));
    fl_set_button_shortcut(obj, scex(_("Right|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 55, 115, 25, idex(_("Center|#c")));
    fl_set_button_shortcut(obj, scex(_("Center|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_valign_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 30, 125, 25, idex(_("Top|#p")));
    fl_set_button_shortcut(obj, scex(_("Top|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_valign_center = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 55, 125, 25, idex(_("Center|#n")));
    fl_set_button_shortcut(obj, scex(_("Center|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_valign_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 80, 120, 25, idex(_("Bottom|#o")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 20, 170, 105, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 200, 20, 130, 105, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 340, 20, 130, 105, _("V. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_column_width = obj = fl_add_input(FL_NORMAL_INPUT, 320, 170, 95, 30, idex(_("Width|#W")));
    fl_set_button_shortcut(obj, scex(_("Width|#W")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_special_alignment = obj = fl_add_input(FL_NORMAL_INPUT, 85, 170, 215, 30, idex(_("Alignment|#A")));
    fl_set_button_shortcut(obj, scex(_("Alignment|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 135, 450, 80, _("Special column"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_cell_options::~FD_form_cell_options()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_cell_options * FormTabular::build_cell_options()
{
  FL_OBJECT *obj;
  FD_form_cell_options *fdui = new FD_form_cell_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 225);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 225, "");
  fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 45, 25, 25, idex(_("Top|#t")));
    fl_set_button_shortcut(obj, scex(_("Top|#t")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 75, 25, 25, idex(_("Bottom|#b")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 65, 60, 25, 25, idex(_("Left|#l")));
    fl_set_button_shortcut(obj, scex(_("Left|#l")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 105, 60, 25, 25, idex(_("Right|#r")));
    fl_set_button_shortcut(obj, scex(_("Right|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 30, 110, 25, idex(_("Left|#e")));
    fl_set_button_shortcut(obj, scex(_("Left|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 80, 115, 25, idex(_("Right|#i")));
    fl_set_button_shortcut(obj, scex(_("Right|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 55, 115, 25, idex(_("Center|#c")));
    fl_set_button_shortcut(obj, scex(_("Center|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_valign_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 30, 125, 25, idex(_("Top|#p")));
    fl_set_button_shortcut(obj, scex(_("Top|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_valign_center = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 55, 125, 25, idex(_("Center|#n")));
    fl_set_button_shortcut(obj, scex(_("Center|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_valign_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 80, 120, 25, idex(_("Bottom|#o")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 20, 170, 105, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 200, 20, 130, 105, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 340, 20, 130, 105, _("V. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_mcolumn_width = obj = fl_add_input(FL_NORMAL_INPUT, 280, 180, 175, 30, idex(_("Width|#W")));
    fl_set_button_shortcut(obj, scex(_("Width|#W")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_special_multialign = obj = fl_add_input(FL_NORMAL_INPUT, 280, 145, 175, 30, idex(_("Alignment|#A")));
    fl_set_button_shortcut(obj, scex(_("Alignment|#A")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_multicolumn = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 140, 130, 25, idex(_("Multicolumn|#M")));
    fl_set_button_shortcut(obj, scex(_("Multicolumn|#M")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_useminipage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 165, 130, 25, idex(_("Use Minipage|#s")));
    fl_set_button_shortcut(obj, scex(_("Use Minipage|#s")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_rotate_cell = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 190, 130, 25, idex(_("Rotate 90院#9")));
    fl_set_button_shortcut(obj, scex(_("Rotate 90院#9")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 135, 150, 80, _("Special Cell"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 180, 135, 290, 80, _("Special Multicolumn"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_longtable_options::~FD_form_longtable_options()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_longtable_options * FormTabular::build_longtable_options()
{
  FL_OBJECT *obj;
  FD_form_longtable_options *fdui = new FD_form_longtable_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 227, "");
  fdui->radio_lt_firsthead = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 55, 90, 25, idex(_("1st Head|#1")));
    fl_set_button_shortcut(obj, scex(_("1st Head|#1")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_lt_head = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 30, 90, 25, idex(_("Head|#H")));
    fl_set_button_shortcut(obj, scex(_("Head|#H")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_lt_foot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 195, 30, 90, 25, idex(_("Foot|#F")));
    fl_set_button_shortcut(obj, scex(_("Foot|#F")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_lt_lastfoot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 195, 55, 90, 25, idex(_("Last Foot|#L")));
    fl_set_button_shortcut(obj, scex(_("Last Foot|#L")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_lt_newpage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 355, 25, 90, 25, idex(_("New Page|#N")));
    fl_set_button_shortcut(obj, scex(_("New Page|#N")), 1);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 20, 150, 75, _("Header"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 185, 20, 150, 75, _("Footer"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 345, 20, 150, 75, _("Special"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

