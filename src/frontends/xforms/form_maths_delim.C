// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"
#include "bmtable.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_delim.h"
#include "FormMathsDelim.h"

FD_form_maths_delim::~FD_form_maths_delim()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_delim * FormMathsDelim::build_maths_delim()
{
  FL_OBJECT *obj;
  FD_form_maths_delim *fdui = new FD_form_maths_delim;

  fdui->form = fl_bgn_form(FL_NO_BOX, 250, 260);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 250, 260, "");
  fdui->bmtable = obj = fl_add_bmtable(FL_PUSH_BUTTON, 40, 70, 170, 140, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);

  fdui->lado = fl_bgn_group();
  {
    char const * const dummy = N_("Right|#R");
    fdui->radio_right = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 40, 40, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_("Left|#L");
    fdui->radio_left = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 40, 10, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 170, 220, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fdui->button_pix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 130, 20, 50, 40, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 90, 220, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 220, 70, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

