// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"
#include "bmtable.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_deco.h"
#include "FormMathsDeco.h"

FD_form_maths_deco::~FD_form_maths_deco()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_deco * FormMathsDeco::build_maths_deco()
{
  FL_OBJECT *obj;
  FD_form_maths_deco *fdui = new FD_form_maths_deco;

  fdui->form = fl_bgn_form(FL_NO_BOX, 160, 150);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 160, 150, "");
  fdui->bmtable = obj = fl_add_bmtable(FL_PUSH_BUTTON, 10, 10, 140, 90, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_RETURN_BUTTON, 30, 110, 100, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

