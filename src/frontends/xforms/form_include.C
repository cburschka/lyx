// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_include.h"
#include "FormInclude.h"

FD_form_include::~FD_form_include()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_include * FormInclude::build_include()
{
  FL_OBJECT *obj;
  FD_form_include *fdui = new FD_form_include;

  fdui->form = fl_bgn_form(FL_NO_BOX, 340, 210);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 210, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 70, 160, 90, "");
  {
    char const * const dummy = N_("Browse|#B");
    fdui->button_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 30, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Don't typeset|#D");
    fdui->check_typeset = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 180, 70, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 120, 170, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 170, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  {
    char const * const dummy = N_("Load|#L");
    fdui->button_load = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 130, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("File name:|#F");
    fdui->input_filename = obj = fl_add_input(FL_NORMAL_INPUT, 10, 30, 210, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Visible space|#s");
    fdui->check_visiblespace = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 180, 100, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  fdui->include_grp = fl_bgn_group();
  {
    char const * const dummy = N_("Verbatim|#V");
    fdui->check_verbatim = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 130, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Use input|#i");
    fdui->check_useinput = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 100, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Use include|#U");
    fdui->check_useinclude = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 70, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

