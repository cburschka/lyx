// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_citation.h"
#include "FormCitation.h"

FD_form_citation::~FD_form_citation()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_citation * FormCitation::build_citation()
{
  FL_OBJECT *obj;
  FD_form_citation *fdui = new FD_form_citation;

  fdui->form = fl_bgn_form(FL_NO_BOX, 435, 665);
  fdui->form->u_vdata = this;
  fdui->box = obj = fl_add_box(FL_UP_BOX, 0, 0, 435, 665, "");
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->citeBrsr = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 30, 180, 300, idex(_("Inset keys|#I")));
    fl_set_button_shortcut(obj, scex(_("Inset keys|#I")), 1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, CITEBRSR);
  fdui->bibBrsr = obj = fl_add_browser(FL_HOLD_BROWSER, 240, 30, 180, 300, idex(_("Bibliography keys|#B")));
    fl_set_button_shortcut(obj, scex(_("Bibliography keys|#B")), 1);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, BIBBRSR);
  fdui->addBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 30, 30, 30, _("@4->"));
    fl_set_button_shortcut(obj, _("#&D"), 1);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, ADD);
  fdui->delBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 65, 30, 30, _("@9+"));
    fl_set_button_shortcut(obj, _("#X"), 1);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, DELETE);
  fdui->upBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 100, 30, 30, _("@8->"));
    fl_set_button_shortcut(obj, _("#&A"), 1);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, UP);
  fdui->downBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 135, 30, 30, _("@2->"));
    fl_set_button_shortcut(obj, _("#&B"), 1);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, DOWN);
  fdui->infoBrsr = obj = fl_add_browser(FL_NORMAL_BROWSER, 10, 360, 410, 80, _("Info"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->style = obj = fl_add_choice(FL_NORMAL_CHOICE, 160, 470, 130, 30, idex(_("Citation style|#s")));
    fl_set_button_shortcut(obj, scex(_("Citation style|#s")), 1);
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->textBefore = obj = fl_add_input(FL_NORMAL_INPUT, 100, 520, 250, 30, idex(_("Text before|#T")));
    fl_set_button_shortcut(obj, scex(_("Text before|#T")), 1);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->textAftr = obj = fl_add_input(FL_NORMAL_INPUT, 100, 570, 250, 30, idex(_("Text after|#e")));
    fl_set_button_shortcut(obj, scex(_("Text after|#e")), 1);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 130, 630, 90, 30, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 630, 90, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 630, 90, 30, idex(_("Cancel|#C^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|#C^[")), 1);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 630, 90, 30, idex(_("Restore|#R")));
    fl_set_button_shortcut(obj, scex(_("Restore|#R")), 1);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

