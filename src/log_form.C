// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "log_form.h"

FD_LaTeXLog *create_form_LaTeXLog(void)
{
  FL_OBJECT *obj;
  FD_LaTeXLog *fdui = (FD_LaTeXLog *) fl_calloc(1, sizeof(FD_LaTeXLog));

  fdui->LaTeXLog = fl_bgn_form(FL_NO_BOX, 470, 380);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 470, 380, "");
  fdui->browser_latexlog = obj = fl_add_browser(FL_NORMAL_BROWSER, 10, 10, 450, 320, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_button(FL_RETURN_BUTTON, 270, 340, 90, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, LatexLogClose, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 370, 340, 90, 30, idex(_("Update|#Uu")));fl_set_button_shortcut(obj, scex(_("Update|#Uu")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, LatexLogUpdate, 0);
  fl_end_form();

  fdui->LaTeXLog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

