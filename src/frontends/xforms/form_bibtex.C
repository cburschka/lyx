// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 160);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 450, 160, "");
  {
    char const * const dummy = N_("Database:|#D");
    fdui->input_database = obj = fl_add_input(FL_NORMAL_INPUT, 90, 10, 245, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 250, 120, 90, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 3);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 120, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 2);
  {
    char const * const dummy = N_("Style:|#S");
    fdui->input_style = obj = fl_add_input(FL_NORMAL_INPUT, 90, 45, 245, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#B");
    fdui->button_database_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 10, 100, 29, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#r");
    fdui->button_style_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 45, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Add bibliography to TOC|#A");
    fdui->check_bibtotoc = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 90, 80, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

