// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "latexoptions.h"

FD_LaTeXOptions *create_form_LaTeXOptions(void)
{
  FL_OBJECT *obj;
  FD_LaTeXOptions *fdui = (FD_LaTeXOptions *) fl_calloc(1, sizeof(FD_LaTeXOptions));

  fdui->LaTeXOptions = fl_bgn_form(FL_NO_BOX, 310, 90);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 310, 90, "");
  fdui->accents = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 10, 310, 30, idex(_("Allow accents on ALL characters|#w")));fl_set_button_shortcut(obj, scex(_("Allow accents on ALL characters|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_button(FL_RETURN_BUTTON, 10, 50, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, LaTeXOptionsOK, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 110, 50, 90, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, LaTeXOptionsApply, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 210, 50, 90, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, LaTeXOptionsCancel, 0);
  fl_end_form();

  //fdui->LaTeXOptions->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

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

  //fdui->LaTeXLog->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

