// File modified by fdfix.sh for use by lyx (with xforms >= 0.86) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_url.h"
#include "FormUrl.h" 

FD_form_url * FormUrl::build_url()
{
  FL_OBJECT *obj;
  FD_form_url *fdui = new FD_form_url;

  fdui->form_url = fl_bgn_form(FL_NO_BOX, 520, 140);
  fdui->form_url->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 520, 140, "");
  fdui->url = obj = fl_add_input(FL_NORMAL_INPUT, 70, 20, 440, 30, _("URL"));
    fl_set_input_shortcut(obj, scex(_("Url|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->name = obj = fl_add_input(FL_NORMAL_INPUT, 70, 60, 440, 30, _("Name"));
    fl_set_input_shortcut(obj, scex(_("Name|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->radio_html = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 70, 100, 230, 30, _("HTML type"));
    fl_set_button_shortcut(obj, scex(_("HTML type|#H")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 410, 100, 100, 30, _("Cancel"));
    fl_set_button_shortcut(obj, _("^["), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormUrlCancelCB, 0);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON, 300, 100, 100, 30, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormUrlOKCB, 0);
  fl_end_form();

  fdui->form_url->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

