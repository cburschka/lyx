// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_index.h"
#include "FormIndex.h"

FD_form_index::~FD_form_index()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_index * FormIndex::build_index()
{
  FL_OBJECT *obj;
  FD_form_index *fdui = new FD_form_index;

  fdui->form = fl_bgn_form(FL_NO_BOX, 520, 100);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 520, 100, "");
  fdui->key = obj = fl_add_input(FL_NORMAL_INPUT, 90, 10, 420, 30, _("Keyword"));
    fl_set_input_shortcut(obj, scex(_("Keyword|#K#k")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 410, 60, 100, 30, _("Cancel"));
    fl_set_button_shortcut(obj, _("^["), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON, 300, 60, 100, 30, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

