// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_bibtex.h"
#include "FormBibtex.h"

FD_form_bibtex::~FD_form_bibtex()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_bibtex * FormBibtex::build_bibtex()
{
  FL_OBJECT *obj;
  FD_form_bibtex *fdui = new FD_form_bibtex;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 170);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 450, 170, "");
  {
    char const * const dummy = N_("Database:|#D");
    fdui->database = obj = fl_add_input(FL_NORMAL_INPUT, 90, 10, 260, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 250, 130, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 3);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 130, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 2);
  {
    char const * const dummy = N_("Style:|#S");
    fdui->style = obj = fl_add_input(FL_NORMAL_INPUT, 90, 50, 260, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#B");
    fdui->database_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 360, 10, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#r");
    fdui->style_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 360, 50, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Add bibliography to TOC|#A");
    fdui->radio_bibtotoc = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 90, 90, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

