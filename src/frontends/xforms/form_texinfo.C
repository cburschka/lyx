// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_texinfo.h"
#include "FormTexinfo.h"

FD_form_texinfo::~FD_form_texinfo()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_texinfo * FormTexinfo::build_texinfo()
{
  FL_OBJECT *obj;
  FD_form_texinfo *fdui = new FD_form_texinfo;

  fdui->form = fl_bgn_form(FL_NO_BOX, 513, 312);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 513, 312, "");
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
  fdui->browser = obj = fl_add_browser(FL_HOLD_BROWSER, 15, 12, 324, 241, "");
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 345, 15, 155, 100, "");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  {
    char const * const dummy = N_("LaTeX Classes|#C");
    fdui->radio_cls = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 350, 20, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("LaTeX Styles|#S");
    fdui->radio_sty = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 350, 50, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("BibTeX Styles|#B");
    fdui->radio_bst = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 350, 80, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 345, 122, 155, 128, "");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  {
    char const * const dummy = N_("Rescan|#R");
    fdui->button_rescan = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 138, 135, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 2);
  {
    char const * const dummy = N_("View|#V");
    fdui->button_view = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 173, 135, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 2);
  {
    char const * const dummy = N_("Show Path|#P");
    fdui->button_fullPath = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 350, 209, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 2);
  {
    char const * const dummy = N_("Run Texhash|#T");
    fdui->button_texhash = obj = fl_add_button(FL_NORMAL_BUTTON, 14, 270, 131, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Close|^[^M");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 409, 270, 90, 30, idex(_(dummy)));
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

