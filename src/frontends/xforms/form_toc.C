// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_toc.h"
#include "FormToc.h"

FD_form_toc::~FD_form_toc()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_toc * FormToc::build_toc()
{
  FL_OBJECT *obj;
  FD_form_toc *fdui = new FD_form_toc;

  fdui->form = fl_bgn_form(FL_NO_BOX, 420, 340);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 340, "");
  fdui->browser = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 10, 400, 280, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  obj = fl_add_button(FL_RETURN_BUTTON, 310, 300, 100, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseHideCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 300, 100, 30, _("Update"));
    fl_set_button_shortcut(obj, scex(_("Update|#U#u")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->type = obj = fl_add_choice(FL_NORMAL_CHOICE, 60, 300, 130, 30, _("Type"));
    fl_set_object_shortcut(obj, scex(_("Type|#T#t")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

