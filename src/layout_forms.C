// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "layout_forms.h"

FD_form_character *create_form_form_character(void)
{
  FL_OBJECT *obj;
  FD_form_character *fdui = (FD_form_character *) fl_calloc(1, sizeof(FD_form_character));

  fdui->form_character = fl_bgn_form(FL_NO_BOX, 300, 415);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 300, 415, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 325, 280, 45, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 265, 280, 45, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 10, 280, 240, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_family = obj = fl_add_choice(FL_NORMAL_CHOICE, 94, 15, 191, 30, idex(_("Family:|#F")));fl_set_button_shortcut(obj, scex(_("Family:|#F")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_series = obj = fl_add_choice(FL_NORMAL_CHOICE, 94, 55, 191, 30, idex(_("Series:|#S")));fl_set_button_shortcut(obj, scex(_("Series:|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_shape = obj = fl_add_choice(FL_NORMAL_CHOICE, 94, 95, 191, 30, idex(_("Shape:|#H")));fl_set_button_shortcut(obj, scex(_("Shape:|#H")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_size = obj = fl_add_choice(FL_NORMAL_CHOICE, 95, 275, 191, 30, idex(_("Size:|#Z")));fl_set_button_shortcut(obj, scex(_("Size:|#Z")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_bar = obj = fl_add_choice(FL_NORMAL_CHOICE, 95, 335, 190, 30, idex(_("Misc:|#M")));fl_set_button_shortcut(obj, scex(_("Misc:|#M")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 375, 80, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, CharacterApplyCB, 0);
  fdui->button_close = obj = fl_add_button(FL_RETURN_BUTTON, 175, 375, 80, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, CharacterCloseCB, 0);
  fdui->choice_color = obj = fl_add_choice(FL_NORMAL_CHOICE, 95, 135, 191, 30, idex(_("Color:|#C")));fl_set_button_shortcut(obj, scex(_("Color:|#C")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->check_toggle_all = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 215, 255, 25, idex(_("Toggle on all these|#T")));fl_set_button_shortcut(obj, scex(_("Toggle on all these|#T")), 1);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 255, 210, 20, _("These are never toggled"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 315, 215, 20, _("These are always toggled"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->choice_language = obj = fl_add_choice(FL_DROPLIST_CHOICE, 95, 175, 190, 30, _("Language:"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fl_end_form();

  fdui->form_character->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_preamble *create_form_form_preamble(void)
{
  FL_OBJECT *obj;
  FD_form_preamble *fdui = (FD_form_preamble *) fl_calloc(1, sizeof(FD_form_preamble));

  fdui->form_preamble = fl_bgn_form(FL_NO_BOX, 380, 420);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 380, 420, "");
  fdui->button_ok = obj = fl_add_button(FL_NORMAL_BUTTON, 50, 380, 100, 30, idex(_("OK|#O")));fl_set_button_shortcut(obj, scex(_("OK|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, PreambleOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 160, 380, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, PreambleApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 380, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, PreambleCancelCB, 0);
  fdui->input_preamble = obj = fl_add_input(FL_MULTILINE_INPUT, 10, 10, 360, 360, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fl_end_form();

  fdui->form_preamble->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

