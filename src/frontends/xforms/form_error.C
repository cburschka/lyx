// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_error.h"
#include "FormError.h"

FD_form_error::~FD_form_error()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_error * FormError::build_error()
{
  FL_OBJECT *obj;
  FD_form_error *fdui = new FD_form_error;

  fdui->form = fl_bgn_form(FL_NO_BOX, 400, 240);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 400, 240, "");
  fdui->message = obj = fl_add_box(FL_FRAME_BOX, 10, 10, 380, 180, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_button(FL_RETURN_BUTTON, 135, 200, 130, 30, idex(_("Close|#C^[^M")));
    fl_set_button_shortcut(obj, scex(_("Close|#C^[^M")), 1);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

