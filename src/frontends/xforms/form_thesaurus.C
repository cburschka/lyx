// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_thesaurus.h"
#include "FormThesaurus.h"

FD_form_thesaurus::~FD_form_thesaurus()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_thesaurus * FormThesaurus::build_thesaurus()
{
  FL_OBJECT *obj;
  FD_form_thesaurus *fdui = new FD_form_thesaurus;

  fdui->form = fl_bgn_form(FL_NO_BOX, 465, 450);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 465, 450, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  {
    char const * const dummy = N_("Replace|^R");
    fdui->button_replace = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 375, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Close|#C^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 180, 410, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->input_entry = obj = fl_add_input(FL_NORMAL_INPUT, 60, 10, 315, 25, _("Entry : "));
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_replace = obj = fl_add_input(FL_NORMAL_INPUT, 75, 375, 260, 30, _("Selection :"));
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Meanings|#M");
    fdui->browser_meanings = obj = fl_add_browser(FL_SELECT_BROWSER, 15, 50, 440, 305, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

