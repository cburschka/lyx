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
  if ( form->visible ) fl_hide_form( form );
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
  {
    char const * const dummy = N_("Close|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 415, 260, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
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
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tabular_options * FormTabular::build_tabular_options()
{
  FL_OBJECT *obj;
  FD_form_tabular_options *fdui = new FD_form_tabular_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 225);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 225, "");
  {
    char const * const dummy = N_("Append Column|#A");
    fdui->button_append_column = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Delete Column|#O");
    fdui->button_delete_column = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Append Row|#p");
    fdui->button_append_row = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 70, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Delete Row|#w");
    fdui->button_delete_row = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 70, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Set Borders|#S");
    fdui->button_set_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 10, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Unset Borders|#U");
    fdui->button_unset_borders = obj = fl_add_button(FL_NORMAL_BUTTON, 140, 10, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Longtable|#L");
    fdui->radio_longtable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 30, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Rotate 90°|#9");
    fdui->radio_rotate_tabular = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 55, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 280, 20, 120, 75, _("Spec. Table"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_column_options::~FD_form_column_options()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_column_options * FormTabular::build_column_options()
{
  FL_OBJECT *obj;
  FD_form_column_options *fdui = new FD_form_column_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 225);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 225, "");
  {
    char const * const dummy = N_("Top|#t");
    fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 45, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#b");
    fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 75, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#l");
    fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 65, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Right|#r");
    fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 105, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#e");
    fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 30, 110, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Right|#i");
    fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 80, 115, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#c");
    fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 55, 115, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Top|#p");
    fdui->radio_valign_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 30, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#n");
    fdui->radio_valign_center = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 55, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#o");
    fdui->radio_valign_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 80, 120, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 20, 170, 105, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 200, 20, 130, 105, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 340, 20, 130, 105, _("V. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Width|#W");
    fdui->input_column_width = obj = fl_add_input(FL_NORMAL_INPUT, 275, 170, 95, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Alignment|#A");
    fdui->input_special_alignment = obj = fl_add_input(FL_NORMAL_INPUT, 40, 170, 215, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 135, 450, 80, _("Special column"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->choice_value_column_width = obj = fl_add_choice(FL_NORMAL_CHOICE, 375, 170, 75, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_cell_options::~FD_form_cell_options()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_cell_options * FormTabular::build_cell_options()
{
  FL_OBJECT *obj;
  FD_form_cell_options *fdui = new FD_form_cell_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 225);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 225, "");
  {
    char const * const dummy = N_("Top|#t");
    fdui->radio_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 45, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#b");
    fdui->radio_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 75, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#l");
    fdui->radio_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 65, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Right|#r");
    fdui->radio_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 105, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#e");
    fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 30, 110, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Right|#i");
    fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 80, 115, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#c");
    fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 205, 55, 115, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Top|#p");
    fdui->radio_valign_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 30, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#n");
    fdui->radio_valign_center = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 55, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#o");
    fdui->radio_valign_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 345, 80, 120, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 20, 170, 105, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 200, 20, 130, 105, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 340, 20, 130, 105, _("V. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Width|#W");
    fdui->input_mcolumn_width = obj = fl_add_input(FL_NORMAL_INPUT, 280, 180, 95, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Alignment|#A");
    fdui->input_special_multialign = obj = fl_add_input(FL_NORMAL_INPUT, 280, 145, 175, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Multicolumn|#M");
    fdui->radio_multicolumn = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 140, 130, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use Minipage|#s");
    fdui->radio_useminipage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 165, 130, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Rotate 90°|#9");
    fdui->radio_rotate_cell = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 190, 130, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 135, 150, 80, _("Special Cell"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 180, 135, 290, 80, _("Special Multicolumn"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->choice_value_mcolumn_width = obj = fl_add_choice(FL_NORMAL_CHOICE, 380, 180, 75, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_longtable_options::~FD_form_longtable_options()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_longtable_options * FormTabular::build_longtable_options()
{
  FL_OBJECT *obj;
  FD_form_longtable_options *fdui = new FD_form_longtable_options;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 227);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 227, "");
  {
    char const * const dummy = N_("1st Head|#1");
    fdui->radio_lt_firsthead = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 55, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Head|#H");
    fdui->radio_lt_head = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 30, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Foot|#F");
    fdui->radio_lt_foot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 195, 30, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Last Foot|#L");
    fdui->radio_lt_lastfoot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 195, 55, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("New Page|#N");
    fdui->radio_lt_newpage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 355, 25, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
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

