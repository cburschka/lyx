// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_forks.h"
#include "FormForks.h"

FD_form_forks::~FD_form_forks()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_forks * FormForks::build_forks()
{
  FL_OBJECT *obj;
  FD_form_forks *fdui = new FD_form_forks;

  fdui->form = fl_bgn_form(FL_NO_BOX, 650, 390);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 650, 390, "");
  {
    char const * const dummy = N_("Forked child processes|#F");
    fdui->browser_children = obj = fl_add_browser(FL_HOLD_BROWSER, 20, 30, 400, 290, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Kill processes|#K");
    fdui->browser_kill = obj = fl_add_browser(FL_HOLD_BROWSER, 510, 30, 125, 290, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_all = obj = fl_add_button(FL_NORMAL_BUTTON, 432, 30, 65, 30, _("All ->"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_add = obj = fl_add_button(FL_NORMAL_BUTTON, 450, 70, 30, 30, _("@->"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_remove = obj = fl_add_button(FL_NORMAL_BUTTON, 450, 110, 30, 30, _("@4->"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 355, 350, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 450, 350, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Close|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 545, 350, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/
