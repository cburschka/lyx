// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_matrix.h"
#include "FormMathsMatrix.h"

FD_form_maths_matrix::~FD_form_maths_matrix()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_matrix * FormMathsMatrix::build_maths_matrix()
{
  FL_OBJECT *obj;
  FD_form_maths_matrix *fdui = new FD_form_maths_matrix;

  fdui->form = fl_bgn_form(FL_NO_BOX, 295, 183);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 295, 183, "");
  fdui->slider_rows = obj = fl_add_valslider(FL_VERT_NICE_SLIDER, 10, 30, 32, 140, _("Rows"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 20);
     fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
  fdui->slider_columns = obj = fl_add_valslider(FL_HOR_NICE_SLIDER, 54, 28, 214, 30, _("Columns "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 20);
     fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
  {
    char const * const dummy = N_("Vertical align|#V");
    fdui->choice_valign = obj = fl_add_choice(FL_NORMAL_CHOICE, 56, 86, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Horizontal align|#H");
    fdui->input_halign = obj = fl_add_input(FL_NORMAL_INPUT, 174, 86, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 44, 140, 80, 30, _("OK  "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 126, 140, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 208, 140, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

