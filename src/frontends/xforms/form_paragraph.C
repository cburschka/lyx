// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_paragraph.h"
#include "FormParagraph.h"

FD_form_paragraph::~FD_form_paragraph()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_paragraph * FormParagraph::build_paragraph()
{
  FL_OBJECT *obj;
  FD_form_paragraph *fdui = new FD_form_paragraph;

  fdui->form = fl_bgn_form(FL_NO_BOX, 500, 450);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 500, 450, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 20, 220, 75, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->group_radio_alignment = fl_bgn_group();
  {
    char const * const dummy = N_("Right|#R");
    fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 15, 30, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Left|#f");
    fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 15, 60, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Block|#c");
    fdui->radio_align_block = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 115, 30, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Center|#n");
    fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 115, 60, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 240, 20, 120, 75, _("Lines"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Above|#b");
    fdui->check_lines_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 245, 30, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Below|#E");
    fdui->check_lines_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 245, 60, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 370, 20, 120, 75, _("Page breaks"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Above|#o");
    fdui->check_pagebreaks_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 375, 30, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Below|#l");
    fdui->check_pagebreaks_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 375, 60, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 110, 480, 105, _("Vertical spaces"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Above:|#v");
    fdui->choice_space_above = obj = fl_add_choice(FL_NORMAL_CHOICE, 80, 130, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_space_above = obj = fl_add_input(FL_NORMAL_INPUT, 190, 130, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Keep|#K");
    fdui->check_space_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 375, 130, 40, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Below:|#w");
    fdui->choice_space_below = obj = fl_add_choice(FL_NORMAL_CHOICE, 80, 170, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_space_below = obj = fl_add_input(FL_NORMAL_INPUT, 190, 170, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Keep|#p");
    fdui->check_space_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 375, 170, 40, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 230, 345, 55, _("Line spacing"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Spacing:|#S");
    fdui->choice_linespacing = obj = fl_add_choice(FL_NORMAL_CHOICE, 80, 245, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_linespacing = obj = fl_add_input(FL_NORMAL_INPUT, 190, 245, 155, 30, "");
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 300, 480, 50, _("Label Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Longest Label:|#g");
    fdui->input_labelwidth = obj = fl_add_input(FL_NORMAL_INPUT, 125, 310, 355, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_West, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 365, 230, 125, 55, _("Indent"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("No Indent|#I");
    fdui->check_noindent = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 375, 245, 115, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 360, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 200, 360, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 300, 360, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 400, 360, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 395, 480, 1, "");
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 10, 405, 480, 35, "");
    fl_set_object_boxtype(obj, FL_NO_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->choice_value_space_above = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 130, 65, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->choice_value_space_below = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 170, 65, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

