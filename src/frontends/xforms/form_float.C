// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_float.h"
#include "FormFloat.h"

FD_form_float::~FD_form_float()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_float * FormFloat::build_float()
{
  FL_OBJECT *obj;
  FD_form_float *fdui = new FD_form_float;

  fdui->form = fl_bgn_form(FL_NO_BOX, 360, 200);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 360, 200, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 100, 340, 50, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 20, 340, 80, _("Placement"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Top of the page|#T");
    fdui->check_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 30, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Bottom of the page|#B");
    fdui->check_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 60, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Page of floats|#P");
    fdui->check_page = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 190, 30, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Here, if possible|#i");
    fdui->check_here = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 190, 60, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Here, definitely|#H");
    fdui->check_here_definitely = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 110, 152, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 160, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 120, 160, 70, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 160, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 280, 160, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/
