// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"
#include "bmtable.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_style.h"
#include "FormMathsStyle.h"

FD_form_maths_style::~FD_form_maths_style()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_style * FormMathsStyle::build_maths_style()
{
  FL_OBJECT *obj;
  FD_form_maths_style *fdui = new FD_form_maths_style;

  fdui->form = fl_bgn_form(FL_NO_BOX, 321, 293);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 321, 293, "");
  fdui->bmtable_font2 = obj = fl_add_bmtable(FL_PUSH_BUTTON, 212, 10, 100, 120, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->bmtable_style1 = obj = fl_add_bmtable(FL_PUSH_BUTTON, 8, 10, 134, 110, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_RETURN_BUTTON, 190, 234, 80, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fdui->bmtable_style2 = obj = fl_add_bmtable(FL_PUSH_BUTTON, 8, 120, 134, 164, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->bmtable_font1 = obj = fl_add_bmtable(FL_PUSH_BUTTON, 152, 10, 60, 200, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_reset = obj = fl_add_button(FL_NORMAL_BUTTON, 212, 170, 100, 40, _("Reset"));
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 13);
  fdui->button_textrm = obj = fl_add_button(FL_NORMAL_BUTTON, 212, 130, 100, 40, _("textrm"));
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 12);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

