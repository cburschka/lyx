// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_ref.h"
#include "FormRef.h"

FD_form_ref::~FD_form_ref()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_ref * FormRef::build_ref()
{
  FL_OBJECT *obj;
  FD_form_ref *fdui = new FD_form_ref;

  fdui->form = fl_bgn_form(FL_NO_BOX, 530, 340);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 530, 340, "");
  fdui->browser = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 10, 270, 240, "");
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_callback(obj, C_FormBaseInputCB, 2);
  fdui->button_update = obj = fl_add_button(FL_NORMAL_BUTTON, 40, 260, 90, 30, idex(_("Update|#U")));
    fl_set_button_shortcut(obj, scex(_("Update|#U")), 1);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 3);
  fdui->sort = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 170, 260, 30, 30, idex(_("Sort|#S")));
    fl_set_button_shortcut(obj, scex(_("Sort|#S")), 1);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 3);
  fdui->name = obj = fl_add_input(FL_NORMAL_INPUT, 370, 10, 150, 40, idex(_("Name:|#N")));
    fl_set_button_shortcut(obj, scex(_("Name:|#N")), 1);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->ref = obj = fl_add_input(FL_NORMAL_INPUT, 370, 60, 150, 40, _("Ref:"));
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->type = obj = fl_add_choice(FL_NORMAL_CHOICE, 340, 140, 140, 40, idex(_("Reference type|#R")));
    fl_set_button_shortcut(obj, scex(_("Reference type|#R")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 4);
  fdui->button_go = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 200, 140, 40, idex(_("Goto reference|#G")));
    fl_set_button_shortcut(obj, scex(_("Goto reference|#G")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 1);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 230, 300, 90, 30, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 430, 300, 90, 30, idex(_("Cancel|#C^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|#C^[")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 300, 90, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 300, 90, 30, idex(_("Restore|#R")));
    fl_set_button_shortcut(obj, scex(_("Restore|#R")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

