// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_copyright.h"
#include "FormCopyright.h"

FD_form_copyright::~FD_form_copyright()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_copyright * FormCopyright::build_copyright()
{
  FL_OBJECT *obj;
  FD_form_copyright *fdui = new FD_form_copyright;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 430);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 450, 430, "");
  fdui->text_copyright = obj = fl_add_text(FL_NORMAL_TEXT, 10, 10, 430, 50, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->text_licence = obj = fl_add_text(FL_NORMAL_TEXT, 10, 70, 430, 110, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->text_disclaimer = obj = fl_add_text(FL_NORMAL_TEXT, 10, 190, 430, 190, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Close|^[^M");
    fdui->button_cancel = obj = fl_add_button(FL_RETURN_BUTTON, 160, 390, 140, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

