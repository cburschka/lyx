// File modified by fdfix.sh for use by lyx (with xforms >= 0.86) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "FormCitation.h"

FD_form_citation * FormCitation::build_citation()
{
  FL_OBJECT *obj;
  FD_form_citation *fdui = new FD_form_citation;

  fdui->form_citation = fl_bgn_form(FL_NO_BOX, 450, 780);
  fdui->form_citation->u_vdata = this;
  fdui->box = obj = fl_add_box(FL_UP_BOX, 0, 0, 450, 780, "");
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->citeBrsr = obj = fl_add_browser(FL_HOLD_BROWSER, 20, 40, 170, 370, _("Inset keys"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormCitationInputCB, CITEBRSR);
  fdui->bibBrsr = obj = fl_add_browser(FL_HOLD_BROWSER, 250, 40, 170, 370, _("Bibliography keys"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormCitationInputCB, BIBBRSR);
  fdui->addBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 40, 40, 40, _("@4->"));
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormCitationInputCB, ADD);
  fdui->delBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 90, 40, 40, _("@9+"));
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormCitationInputCB, DELETE);
  fdui->upBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 140, 40, 40, _("@8->"));
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormCitationInputCB, UP);
  fdui->downBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 190, 40, 40, _("@2->"));
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormCitationInputCB, DOWN);
  fdui->infoBrsr = obj = fl_add_browser(FL_NORMAL_BROWSER, 20, 440, 400, 110, _("Info"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->style = obj = fl_add_choice(FL_NORMAL_CHOICE, 160, 570, 130, 30, _("Citation style"));
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->textBefore = obj = fl_add_input(FL_NORMAL_INPUT, 100, 620, 250, 30, _("Text before"));
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->textAftr = obj = fl_add_input(FL_NORMAL_INPUT, 100, 660, 250, 30, _("Text after"));
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON, 190, 730, 110, 40, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormCitationOKCB, 0);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 730, 110, 40, _("Cancel"));
    fl_set_button_shortcut(obj, _("^["), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormCitationCancelCB, 0);
  fl_end_form();

  fdui->form_citation->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

