// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_filedialog.h"
#include "FormFiledialog.h"

FD_form_filedialog::~FD_form_filedialog()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_filedialog * FileDialog::Private::build_filedialog()
{
  FL_OBJECT *obj;
  FD_form_filedialog *fdui = new FD_form_filedialog;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 380);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 440, 380, "");
  fdui->FileInfo = obj = fl_add_text(FL_NORMAL_TEXT, 10, 260, 420, 30, "");
    fl_set_object_boxtype(obj, FL_SHADOW_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  {
    char const * const dummy = N_("Directory:|#D");
    fdui->DirBox = obj = fl_add_input(FL_NORMAL_INPUT, 100, 10, 330, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  {
    char const * const dummy = N_("Pattern:|#P");
    fdui->PatBox = obj = fl_add_input(FL_NORMAL_INPUT, 100, 40, 330, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->List = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 80, 320, 170, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  {
    char const * const dummy = N_("Filename:|#F");
    fdui->Filename = obj = fl_add_input(FL_NORMAL_INPUT, 100, 300, 330, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  {
    char const * const dummy = N_("Rescan|#R#r");
    fdui->Rescan = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 80, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  {
    char const * const dummy = N_("Home|#H#h");
    fdui->Home = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 120, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  {
    char const * const dummy = N_("User1|#1");
    fdui->User1 = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 160, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  {
    char const * const dummy = N_("User2|#2");
    fdui->User2 = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 200, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->Ready = obj = fl_add_button(FL_RETURN_BUTTON, 220, 340, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->Cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 340, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->timer = obj = fl_add_timer(FL_HIDDEN_TIMER, 10, 350, 20, 20, "");
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

