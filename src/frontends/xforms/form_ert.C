// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_ert.h"
#include "FormERT.h"

FD_form_ert::~FD_form_ert()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_ert * FormERT::build_ert()
{
  FL_OBJECT *obj;
  FD_form_ert *fdui = new FD_form_ert;

  fdui->form = fl_bgn_form(FL_NO_BOX, 430, 170);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 430, 170, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 20, 410, 100, _("Status"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
 fl_bgn_group();
  {
    char const * const dummy = N_("Open|#O");
    fdui->radio_open = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 19, 30, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Collapsed|#C");
    fdui->radio_collapsed = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 19, 60, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Inlined View|#I");
    fdui->radio_inlined = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 19, 90, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 130, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 130, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 130, 130, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

