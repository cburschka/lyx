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

  fdui->form = fl_bgn_form(FL_NO_BOX, 800, 510);
  fdui->form->u_vdata = this;
  fdui->box = obj = fl_add_box(FL_UP_BOX, 0, 0, 800, 510, "");
  {
    char const * const dummy = N_("Inset keys|#I");
    fdui->browser_cite = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 30, 165, 460, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_NorthWest, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Bibliography keys|#B");
    fdui->browser_bib = obj = fl_add_browser(FL_HOLD_BROWSER, 225, 30, 165, 460, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_North, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_add = obj = fl_add_button(FL_NORMAL_BUTTON, 185, 30, 30, 30, _("@4->"));
    fl_set_button_shortcut(obj, _("#&D"), 1);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_del = obj = fl_add_button(FL_NORMAL_BUTTON, 185, 65, 30, 30, _("@9+"));
    fl_set_button_shortcut(obj, _("#X"), 1);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_up = obj = fl_add_button(FL_NORMAL_BUTTON, 185, 100, 30, 30, _("@8->"));
    fl_set_button_shortcut(obj, _("#&A"), 1);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_down = obj = fl_add_button(FL_NORMAL_BUTTON, 185, 135, 30, 30, _("@2->"));
    fl_set_button_shortcut(obj, _("#&B"), 1);
    fl_set_object_gravity(obj, FL_North, FL_North);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->browser_info = obj = fl_add_browser(FL_NORMAL_BROWSER, 410, 30, 370, 90, _("Info"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->frame_search = obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 410, 140, 370, 120, _("Search"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
  fdui->input_search = obj = fl_add_input(FL_NORMAL_INPUT, 430, 150, 340, 30, "");
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->button_search_type = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 430, 190, 30, 30, _("Regular Expression"));
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
  fdui->button_search_case = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 430, 220, 30, 30, _("Case sensitive"));
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
  {
    char const * const dummy = N_("Previous|#P");
    fdui->button_previous = obj = fl_add_button(FL_NORMAL_BUTTON, 680, 190, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Next|#N");
    fdui->button_next = obj = fl_add_button(FL_NORMAL_BUTTON, 680, 225, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->frame_style = obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 410, 280, 370, 80, _("Citation style"));
    fl_set_object_shortcut(obj, _("frame_style"), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->choice_style = obj = fl_add_choice(FL_NORMAL_CHOICE, 550, 290, 130, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Full author list|#F");
    fdui->button_full_author_list = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 530, 320, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Force upper case|#u");
    fdui->button_force_uppercase = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 730, 320, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Text before|#T");
    fdui->input_before = obj = fl_add_input(FL_NORMAL_INPUT, 530, 375, 250, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Text after|#e");
    fdui->input_after = obj = fl_add_input(FL_NORMAL_INPUT, 530, 410, 250, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 410, 460, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 510, 460, 90, 30, _("OK"));
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 605, 460, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 700, 460, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

