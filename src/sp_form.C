// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "sp_form.h"

FD_form_spell_options *create_form_form_spell_options(void)
{
  FL_OBJECT *obj;
  FD_form_spell_options *fdui = (FD_form_spell_options *) fl_calloc(1, sizeof(FD_form_spell_options));

  fdui->form_spell_options = fl_bgn_form(FL_NO_BOX, 340, 400);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 400, "");
    fl_set_object_lsize(obj, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 320, 110, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 150, 320, 200, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);

  fdui->lang_buts = fl_bgn_group();
  fdui->buflang = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 320, 30, idex(_("Use language of document|#D")));fl_set_button_shortcut(obj, scex(_("Use language of document|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->altlang = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 320, 30, idex(_("Use alternate language:|#U")));fl_set_button_shortcut(obj, scex(_("Use alternate language:|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  fdui->altlang_input = obj = fl_add_input(FL_NORMAL_INPUT, 40, 90, 240, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->compounds = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 160, 320, 30, idex(_("Treat run-together words as legal|#T")));fl_set_button_shortcut(obj, scex(_("Treat run-together words as legal|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->inpenc = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 190, 320, 30, idex(_("Input Encoding switch to ispell|#I")));fl_set_button_shortcut(obj, scex(_("Input Encoding switch to ispell|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 360, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SpellOptionsOKCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 360, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SpellOptionsCancelCB, 0);
  fdui->perdict = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 220, 320, 30, idex(_("Use alternate personal dictionary:|#P")));fl_set_button_shortcut(obj, scex(_("Use alternate personal dictionary:|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->esc_chars = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 280, 320, 30, idex(_("Extra special chars allowed in words:|#E")));fl_set_button_shortcut(obj, scex(_("Extra special chars allowed in words:|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->perdict_input = obj = fl_add_input(FL_NORMAL_INPUT, 40, 250, 240, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->esc_chars_input = obj = fl_add_input(FL_NORMAL_INPUT, 40, 310, 240, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 90, 20, _("Dictionary"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 30, 140, 70, 20, _("Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON, 120, 360, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SpellOptionsApplyCB, 0);
  fl_end_form();

  fdui->form_spell_options->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_spell_check *create_form_form_spell_check(void)
{
  FL_OBJECT *obj;
  FD_form_spell_check *fdui = (FD_form_spell_check *) fl_calloc(1, sizeof(FD_form_spell_check));

  fdui->form_spell_check = fl_bgn_form(FL_NO_BOX, 540, 280);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 540, 280, "");
    fl_set_object_lsize(obj, 0);
  fdui->text = obj = fl_add_text(FL_NORMAL_TEXT, 80, 10, 220, 30, "");
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->input = obj = fl_add_input(FL_NORMAL_INPUT, 80, 40, 220, 30, _("Replace"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->browser = obj = fl_add_browser(FL_SELECT_BROWSER, 80, 70, 220, 150, _("Near\nMisses"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fdui->options = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 210, 220, 30, idex(_("Spellchecker Options...|#O")));fl_set_button_shortcut(obj, scex(_("Spellchecker Options...|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->start = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 10, 220, 30, idex(_("Start spellchecking|#S")));fl_set_button_shortcut(obj, scex(_("Start spellchecking|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->insert = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 50, 220, 30, idex(_("Insert in personal dictionary|#I")));fl_set_button_shortcut(obj, scex(_("Insert in personal dictionary|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->ignore = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 110, 220, 30, idex(_("Ignore word|#g")));fl_set_button_shortcut(obj, scex(_("Ignore word|#g")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->accept = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 80, 220, 30, idex(_("Accept word in this session|#A")));fl_set_button_shortcut(obj, scex(_("Accept word in this session|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->stop = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 180, 220, 30, idex(_("Stop spellchecking|#T")));fl_set_button_shortcut(obj, scex(_("Stop spellchecking|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->done = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 240, 220, 30, idex(_("Close Spellchecker|#C^[")));fl_set_button_shortcut(obj, scex(_("Close Spellchecker|#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  // xgettext:no-c-format
  obj = fl_add_box(FL_NO_BOX, 10, 250, 50, 20, _("0 %"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  // xgettext:no-c-format
  obj = fl_add_box(FL_NO_BOX, 250, 250, 50, 20, _("100 %"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->replace = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 140, 220, 30, idex(_("Replace word|#R")));fl_set_button_shortcut(obj, scex(_("Replace word|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->slider = obj = fl_add_slider(FL_HOR_FILL_SLIDER, 10, 230, 290, 20, "");
    fl_set_object_color(obj, FL_BLUE, FL_COL1);
    fl_set_object_lsize(obj, 0);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_end_form();

  fdui->form_spell_check->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

