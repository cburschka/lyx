// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_space.h"
#include "FormMathsSpace.h"

FD_form_maths_space::~FD_form_maths_space()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_space * FormMathsSpace::build_maths_space()
{
  FL_OBJECT *obj;
  FD_form_maths_space *fdui = new FD_form_maths_space;

  fdui->form = fl_bgn_form(FL_NO_BOX, 280, 150);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 280, 150, "");

  fdui->spaces = fl_bgn_group();
  {
    char const * const dummy = N_("Thin|#T");
    fdui->radio_thin = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 10, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 1);
  {
    char const * const dummy = N_("Medium|#M");
    fdui->radio_medium = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 2);
  {
    char const * const dummy = N_("Thick|#H");
    fdui->radio_thick = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 70, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 3);
  {
    char const * const dummy = N_("Negative|#N");
    fdui->radio_negative = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 10, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Quadratin|#Q");
    fdui->radio_quadratin = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 4);
  {
    char const * const dummy = N_("2Quadratin|#2");
    fdui->radio_twoquadratin = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 70, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 5);
  fl_end_group();

  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 110, 80, 30, _("OK "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 190, 110, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 100, 110, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

