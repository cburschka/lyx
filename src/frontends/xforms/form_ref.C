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
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_ref * FormRef::build_ref()
{
  FL_OBJECT *obj;
  FD_form_ref *fdui = new FD_form_ref;

  fdui->form = fl_bgn_form(FL_NO_BOX, 530, 340);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 530, 340, "");
  fdui->browser = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 10, 270, 280, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormCommandInputCB, 2);
  fdui->update = obj = fl_add_button(FL_NORMAL_BUTTON, 40, 300, 90, 30, _("Update"));
    fl_set_button_shortcut(obj, scex(_("Update|#U#u")), 1);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormCommandInputCB, 3);
  fdui->sort = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 170, 300, 30, 30, _("Sort"));
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormCommandInputCB, 3);
  fdui->name = obj = fl_add_input(FL_NORMAL_INPUT, 370, 10, 150, 40, _("Name:"));
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->ref = obj = fl_add_input(FL_NORMAL_INPUT, 370, 60, 150, 40, _("Reference:"));
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->type = obj = fl_add_choice(FL_NORMAL_CHOICE, 340, 140, 140, 40, _("Reference type"));
    fl_set_object_shortcut(obj, scex(_("Reference type|#t")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormCommandInputCB, 4);
  fdui->go = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 200, 140, 40, _("Goto reference"));
    fl_set_button_shortcut(obj, scex(_("Goto reference|#G")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormCommandInputCB, 1);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON, 330, 300, 90, 30, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormCommandOKCB, 0);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 430, 300, 90, 30, _("Cancel"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormCommandCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

