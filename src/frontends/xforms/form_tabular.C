// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 510, 325);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 510, 325, "");
  fdui->tabfolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 0, 0, 505, 250, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Close|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 415, 290, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_tabular_column = obj = fl_add_input(FL_NORMAL_INPUT, 65, 290, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->input_tabular_row = obj = fl_add_input(FL_NORMAL_INPUT, 5, 290, 60, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 5, 255, 500, 30, "");
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
    fdui->check_longtable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 30, 90, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Rotate 90°|#9");
    fdui->check_rotate_tabular = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 280, 55, 90, 25, idex(_(dummy)));
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 325, 20, 170, 195, _("Fixed Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 20, 170, 105, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 200, 20, 115, 105, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 140, 295, 75, _("Special column"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_(" |#W");
    fdui->input_column_width = obj = fl_add_input(FL_NORMAL_INPUT, 335, 55, 75, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Top|#t");
    fdui->check_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 45, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#b");
    fdui->check_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 75, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#l");
    fdui->check_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 65, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Right|#r");
    fdui->check_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 105, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
 fl_bgn_group();
  {
    char const * const dummy = N_("Left|#e");
    fdui->radio_align_left = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 35, 105, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Right|#i");
    fdui->radio_align_right = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 85, 105, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#c");
    fdui->radio_align_center = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 60, 105, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

 fl_bgn_group();
  {
    char const * const dummy = N_("Top|#p");
    fdui->radio_valign_top = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 330, 135, 155, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Center|#n");
    fdui->radio_valign_center = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 330, 160, 155, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#o");
    fdui->radio_valign_bottom = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 330, 185, 155, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("LaTeX Argument|#A");
    fdui->input_special_alignment = obj = fl_add_input(FL_NORMAL_INPUT, 30, 180, 275, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_(" |#L");
    fdui->choice_value_column_width = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 55, 75, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 335, 110, 155, 25, _("V. Alignment"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 135, 140, 80, _("Special Cell"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 160, 135, 175, 80, _("Special Multicolumn"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 345, 20, 150, 195, _("Fixed Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 190, 20, 145, 100, _("H. Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 20, 170, 100, _("Borders"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Top|#t");
    fdui->check_border_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 75, 45, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#b");
    fdui->check_border_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 75, 75, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_BOTTOM_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#l");
    fdui->check_border_left = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 55, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Right|#r");
    fdui->check_border_right = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 95, 60, 25, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
 fl_bgn_group();
  {
    char const * const dummy = N_("Left|#e");
    fdui->radio_align_left = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 35, 110, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Right|#i");
    fdui->radio_align_right = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 85, 115, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#c");
    fdui->radio_align_center = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 60, 115, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

 fl_bgn_group();
  {
    char const * const dummy = N_("Top|#p");
    fdui->radio_valign_top = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 350, 135, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Center|#n");
    fdui->radio_valign_center = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 350, 160, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#o");
    fdui->radio_valign_bottom = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 350, 185, 125, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_(" |#W");
    fdui->input_mcolumn_width = obj = fl_add_input(FL_NORMAL_INPUT, 350, 50, 75, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("LaTeX Argument|#A");
    fdui->input_special_multialign = obj = fl_add_input(FL_NORMAL_INPUT, 165, 180, 165, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Multicolumn|#M");
    fdui->check_multicolumn = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 140, 130, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use Minipage|#s");
    fdui->check_useminipage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 165, 130, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Rotate 90°|#9");
    fdui->check_rotate_cell = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 190, 130, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_(" |#L");
    fdui->choice_value_mcolumn_width = obj = fl_add_choice(FL_NORMAL_CHOICE, 430, 50, 60, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 350, 110, 135, 25, _("V. Alignment"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 225);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 225, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 15, 485, 30, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 150, 485, 35, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 115, 485, 35, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 80, 485, 35, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 45, 485, 35, "");
  fdui->check_lt_firsthead = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 135, 85, 60, 25, _("On"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_lt_head = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 135, 50, 60, 25, _("On"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_lt_foot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 135, 120, 60, 25, _("On"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_lt_lastfoot = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 135, 155, 60, 25, _("On"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Page break on the current row|#B");
    fdui->check_lt_newpage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 190, 245, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_head_2border_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 205, 50, 85, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_head_2border_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 50, 75, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 50, 120, 25, _("Header"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 85, 120, 25, _("First Header"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 120, 120, 25, _("Footer"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 15, 155, 120, 25, _("Last Footer"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->check_1head_2border_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 205, 85, 85, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_foot_2border_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 205, 120, 85, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_lastfoot_2border_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 205, 155, 85, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_1head_2border_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 85, 75, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_foot_2border_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 120, 75, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_lastfoot_2border_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 155, 75, 25, _("Double"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_1head_empty = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 390, 85, 75, 25, _("Is Empty"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_lastfoot_empty = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 390, 155, 75, 25, _("Is Empty"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 205, 20, 85, 20, _("Border Above"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 300, 20, 85, 20, _("Border Below"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 390, 20, 85, 20, _("Contents"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 135, 20, 60, 20, _("Status"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

