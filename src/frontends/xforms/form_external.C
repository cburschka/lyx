// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_external.h"
#include "FormExternal.h"

FD_form_external::~FD_form_external()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_external * FormExternal::build_external()
{
  FL_OBJECT *obj;
  FD_form_external *fdui = new FD_form_external;

  fdui->form = fl_bgn_form(FL_NO_BOX, 560, 310);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 560, 310, "");
  {
    char const * const dummy = N_("Template|#t");
    fdui->choice_template = obj = fl_add_choice(FL_NORMAL_CHOICE, 130, 10, 300, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->browser_helptext = obj = fl_add_browser(FL_NORMAL_BROWSER, 130, 50, 300, 130, "");
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  {
    char const * const dummy = N_("File|#F");
    fdui->input_filename = obj = fl_add_input(FL_NORMAL_INPUT, 130, 190, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#B");
    fdui->button_filenamebrowse = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 190, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Parameters|#P");
    fdui->input_parameters = obj = fl_add_input(FL_NORMAL_INPUT, 130, 230, 300, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Edit file|#E");
    fdui->button_edit = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 50, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("View result|#V");
    fdui->button_view = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 90, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Update result|#U");
    fdui->button_update = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 130, 110, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 255, 270, 90, 30, _("OK"));
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 270, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|#C^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 455, 270, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

