// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_bibitem.h"
#include "FormBibitem.h"

FD_form_bibitem::~FD_form_bibitem()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_bibitem * FormBibitem::build_bibitem()
{
  FL_OBJECT *obj;
  FD_form_bibitem *fdui = new FD_form_bibitem;

  fdui->form = fl_bgn_form(FL_NO_BOX, 220, 130);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 220, 130, "");
  {
    char const * const dummy = N_("Key:|#K");
    fdui->input_key = obj = fl_add_input(FL_NORMAL_INPUT, 80, 10, 130, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 20, 90, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 3);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 120, 90, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 2);
  {
    char const * const dummy = N_("Label:|#L");
    fdui->input_label = obj = fl_add_input(FL_NORMAL_INPUT, 80, 50, 130, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

