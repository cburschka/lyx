// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_toc.h"
#include "FormToc.h"

FD_form_toc::~FD_form_toc()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_toc * FormToc::build_toc()
{
  FL_OBJECT *obj;
  FD_form_toc *fdui = new FD_form_toc;

  fdui->form = fl_bgn_form(FL_NO_BOX, 420, 340);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 340, "");
  fdui->browser_toc = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 10, 400, 280, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Update|#U");
    fdui->button_update = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 300, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Type|#T");
    fdui->choice_toc_type = obj = fl_add_choice(FL_NORMAL_CHOICE, 60, 300, 130, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Close|^[^M");
    fdui->button_cancel = obj = fl_add_button(FL_RETURN_BUTTON, 310, 300, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

