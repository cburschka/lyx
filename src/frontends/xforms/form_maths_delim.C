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

  fdui->form = fl_bgn_form(FL_NO_BOX, 250, 307);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 250, 307, "");
  fdui->bmtable = obj = fl_add_bmtable(FL_PUSH_BUTTON, 42, 112, 170, 140, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 168, 268, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fdui->button_pix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 100, 58, 50, 40, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 90, 268, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 12, 268, 70, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
 fl_bgn_group();
  {
    char const * const dummy = N_("()\nBoth|#B");
    fdui->radio_both = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 80, 8, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_(")\nRight|#R");
    fdui->radio_right = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 162, 8, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_("(\nLeft|#L");
    fdui->radio_left = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 8, 8, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

