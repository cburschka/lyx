// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "form_url.h"

FD_form_url *create_form_form_url(void)
{
  FL_OBJECT *obj;
  FD_form_url *fdui = (FD_form_url *) fl_calloc(1, sizeof(FD_form_url));

  fdui->form_url = fl_bgn_form(FL_NO_BOX, 530, 150);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 530, 150, "");
  fdui->url_name = obj = fl_add_input(FL_NORMAL_INPUT, 50, 20, 460, 30, _("Url"));
    fl_set_input_shortcut(obj, scex(_("Url|#U")), 1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->name_name = obj = fl_add_input(FL_NORMAL_INPUT, 50, 60, 460, 30, _("Name"));
    fl_set_input_shortcut(obj, scex(_("Name|#N")), 1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->radio_html = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 50, 110, 250, 30, _("HTML type"));
    fl_set_button_shortcut(obj, scex(_("HTML type|#H")), 1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->button_close = obj = fl_add_button(FL_RETURN_BUTTON, 410, 110, 100, 30, _("Close"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_InsetUrl_CloseUrlCB, 0);
  fl_end_form();

  //fdui->form_url->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

