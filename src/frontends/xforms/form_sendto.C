// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_sendto.h"
#include "FormSendto.h"

FD_form_sendto::~FD_form_sendto()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_sendto * FormSendto::build_sendto()
{
  FL_OBJECT *obj;
  FD_form_sendto *fdui = new FD_form_sendto;

  fdui->form = fl_bgn_form(FL_NO_BOX, 360, 410);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 360, 410, "");
  {
    char const * const dummy = N_("Export format|#E");
    fdui->browser_formats = obj = fl_add_browser(FL_HOLD_BROWSER, 15, 30, 145, 250, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Command|#C");
    fdui->input_command = obj = fl_add_input(FL_NORMAL_INPUT, 10, 315, 335, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Help:|#H");
    fdui->choice_help = obj = fl_add_choice(FL_NORMAL_CHOICE, 215, 250, 130, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 65, 370, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 160, 370, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 255, 370, 90, 30, idex(_(dummy)));
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

