// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_browser.h"
#include "FormBrowser.h"

FD_form_browser::~FD_form_browser()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_browser * FormBrowser::build_browser()
{
  FL_OBJECT *obj;
  FD_form_browser *fdui = new FD_form_browser;

  fdui->form = fl_bgn_form(FL_NO_BOX, 470, 380);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 470, 380, "");
  fdui->browser = obj = fl_add_browser(FL_NORMAL_BROWSER, 10, 10, 450, 320, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  {
    char const * const dummy = N_("Close|^[^M");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 370, 340, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  {
    char const * const dummy = N_("Update|#Uu");
    fdui->button_update = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 340, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

