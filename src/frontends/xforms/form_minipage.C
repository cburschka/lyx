// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_minipage.h"
#include "FormMinipage.h"

FD_form_minipage::~FD_form_minipage()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_minipage * FormMinipage::build_minipage()
{
  FL_OBJECT *obj;
  FD_form_minipage *fdui = new FD_form_minipage;

  fdui->form = fl_bgn_form(FL_NO_BOX, 430, 170);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 430, 170, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 20, 190, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 20, 20, 200, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  {
    char const * const dummy = N_("Length|#L");
    fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 100, 30, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    // xgettext:no-c-format
    char const * const dummy = N_("or %|#o");
    fdui->input_widthp = obj = fl_add_input(FL_INT_INPUT, 100, 70, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 30, 10, 100, 20, _("Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 10, 140, 20, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->group_alignment = fl_bgn_group();
  {
    char const * const dummy = N_("Top|#T");
    fdui->radio_top = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 249, 30, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Middle|#d");
    fdui->radio_middle = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 249, 60, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bottom|#B");
    fdui->radio_bottom = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 249, 90, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 130, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 130, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 130, 130, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 130, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedRestoreCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

