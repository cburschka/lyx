// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_search.h"
#include "FormSearch.h"

FD_form_search::~FD_form_search()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_search * FormSearch::build_search()
{
  FL_OBJECT *obj;
  FD_form_search *fdui = new FD_form_search;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 160);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 440, 160, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 300, 10, 130, 80, "");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  {
    char const * const dummy = N_("Find|#n");
    fdui->input_search = obj = fl_add_input(FL_NORMAL_INPUT, 110, 10, 180, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Replace with|#W");
    fdui->input_replace = obj = fl_add_input(FL_NORMAL_INPUT, 110, 40, 180, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Forwards >|#F^s");
    fdui->findnext = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 80, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_(" < Backwards|#B^r");
    fdui->findprev = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 80, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Replace|#R#r");
    fdui->replace = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 120, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Close|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 120, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  {
    char const * const dummy = N_("Case sensitive|#s#S");
    fdui->casesensitive = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 20, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Match word|#M#m");
    fdui->matchword = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 50, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Replace All|#A#a");
    fdui->replaceall = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 120, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

