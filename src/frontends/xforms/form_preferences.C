// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_preferences.h"
#include "FormPreferences.h"

FD_form_preferences::~FD_form_preferences()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_preferences * FormPreferences::build_preferences()
{
  FL_OBJECT *obj;
  FD_form_preferences *fdui = new FD_form_preferences;

  fdui->form = fl_bgn_form(FL_NO_BOX, 470, 500);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 470, 500, "");
  fdui->tabfolder_prefs = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 5, 10, 455, 375, "");
    fl_set_object_boxtype(obj, FL_FLAT_BOX);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 390, 90, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 370, 390, 90, 30, idex(_("Cancel|C#C^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|C#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 170, 390, 90, 30, _("Save"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 5, 390, 90, 30, idex(_("Restore|#R")));
    fl_set_button_shortcut(obj, scex(_("Restore|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 0, 425, 470, 1, "");
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 5, 430, 460, 70, "");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_outer_tab::~FD_form_outer_tab()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_outer_tab * FormPreferences::build_outer_tab()
{
  FL_OBJECT *obj;
  FD_form_outer_tab *fdui = new FD_form_outer_tab;

  fdui->form = fl_bgn_form(FL_NO_BOX, 455, 375);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 455, 375, "");
  fdui->tabfolder_outer = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 0, 0, 455, 375, "");
    fl_set_object_boxtype(obj, FL_FLAT_BOX);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_screen_fonts::~FD_form_screen_fonts()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_screen_fonts * FormPreferences::build_screen_fonts()
{
  FL_OBJECT *obj;
  FD_form_screen_fonts *fdui = new FD_form_screen_fonts;

  fdui->form = fl_bgn_form(FL_NO_BOX, 455, 375);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 455, 375, "");
  fdui->input_roman = obj = fl_add_input(FL_NORMAL_INPUT, 210, 5, 200, 30, _("Roman"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_sans = obj = fl_add_input(FL_NORMAL_INPUT, 210, 35, 200, 30, _("Sans Serif"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_typewriter = obj = fl_add_input(FL_NORMAL_INPUT, 210, 65, 200, 30, _("Typewriter"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->counter_zoom = obj = fl_add_counter(FL_SIMPLE_COUNTER, 345, 130, 65, 30, idex(_("Zoom %|#Z")));
    fl_set_button_shortcut(obj, scex(_("Zoom %|#Z")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 999);
    fl_set_counter_value(obj, 150);
    fl_set_counter_step(obj, 1, 1);
  fdui->check_scalable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 60, 143, 33, 31, _("Use scalable fonts"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_screen_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 210, 95, 200, 30, _("Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_script = obj = fl_add_input(FL_FLOAT_INPUT, 200, 199, 70, 30, _("script"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_footnote = obj = fl_add_input(FL_FLOAT_INPUT, 340, 198, 70, 30, _("footnote"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_large = obj = fl_add_input(FL_FLOAT_INPUT, 340, 228, 70, 30, _("large"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_largest = obj = fl_add_input(FL_FLOAT_INPUT, 200, 259, 70, 30, _("largest"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_huge = obj = fl_add_input(FL_FLOAT_INPUT, 340, 258, 70, 30, _("huge"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_normal = obj = fl_add_input(FL_FLOAT_INPUT, 200, 229, 70, 30, _("normal"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->counter_dpi = obj = fl_add_counter(FL_SIMPLE_COUNTER, 345, 162, 65, 30, idex(_("Screen DPI|#D")));
    fl_set_button_shortcut(obj, scex(_("Screen DPI|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 999);
    fl_set_counter_value(obj, 150);
    fl_set_counter_step(obj, 1, 1);
  fdui->input_tiny = obj = fl_add_input(FL_FLOAT_INPUT, 60, 198, 70, 30, _("tiny"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_larger = obj = fl_add_input(FL_FLOAT_INPUT, 60, 258, 70, 30, _("larger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_small = obj = fl_add_input(FL_FLOAT_INPUT, 60, 228, 70, 30, _("small"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_huger = obj = fl_add_input(FL_FLOAT_INPUT, 60, 288, 70, 30, _("huger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_outputs_misc::~FD_form_outputs_misc()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_outputs_misc * FormPreferences::build_outputs_misc()
{
  FL_OBJECT *obj;
  FD_form_outputs_misc *fdui = new FD_form_outputs_misc;

  fdui->form = fl_bgn_form(FL_NO_BOX, 455, 375);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 455, 375, "");
  fdui->counter_line_len = obj = fl_add_counter(FL_SIMPLE_COUNTER, 295, 50, 120, 30, idex(_("Ascii line length|#A")));
    fl_set_button_shortcut(obj, scex(_("Ascii line length|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 120);
    fl_set_counter_value(obj, 75);
    fl_set_counter_step(obj, 1, 1);
  fdui->input_tex_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 216, 95, 200, 30, idex(_("TeX encoding|#T")));
    fl_set_button_shortcut(obj, scex(_("TeX encoding|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_default_papersize = obj = fl_add_choice(FL_NORMAL_CHOICE, 219, 142, 199, 29, idex(_("Default paper size|#p")));
    fl_set_button_shortcut(obj, scex(_("Default paper size|#p")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_ascii_roff = obj = fl_add_input(FL_NORMAL_INPUT, 121, 209, 285, 30, idex(_("ascii roff|#r")));
    fl_set_button_shortcut(obj, scex(_("ascii roff|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_checktex = obj = fl_add_input(FL_NORMAL_INPUT, 121, 250, 285, 30, idex(_("checktex|#c")));
    fl_set_button_shortcut(obj, scex(_("checktex|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 12, 186, 406, 120, _("Outside code interaction"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_spellchecker::~FD_form_spellchecker()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_spellchecker * FormPreferences::build_spellchecker()
{
  FL_OBJECT *obj;
  FD_form_spellchecker *fdui = new FD_form_spellchecker;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  fdui->choice_spell_command = obj = fl_add_choice(FL_NORMAL_CHOICE, 141, 47, 171, 34, idex(_("Spell command|#S")));
    fl_set_button_shortcut(obj, scex(_("Spell command|#S")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_alt_lang = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 208, 100, 30, 30, idex(_("Use alternative language|#a")));
    fl_set_button_shortcut(obj, scex(_("Use alternative language|#a")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_alt_lang = obj = fl_add_input(FL_NORMAL_INPUT, 239, 100, 170, 30, "");
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_escape_chars = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 208, 130, 30, 30, idex(_("Use escape characters|#e")));
    fl_set_button_shortcut(obj, scex(_("Use escape characters|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_escape_chars = obj = fl_add_input(FL_NORMAL_INPUT, 239, 130, 170, 30, "");
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_personal_dict = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 208, 160, 30, 30, idex(_("Use personal dictionary|#d")));
    fl_set_button_shortcut(obj, scex(_("Use personal dictionary|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_personal_dict = obj = fl_add_input(FL_NORMAL_INPUT, 239, 160, 170, 30, "");
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_personal_dict = obj = fl_add_button(FL_NORMAL_BUTTON, 239, 190, 89, 30, idex(_("Browse...|#B")));
    fl_set_button_shortcut(obj, scex(_("Browse...|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_compound_words = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 130, 230, 30, 30, idex(_("Accept compound words|#w")));
    fl_set_button_shortcut(obj, scex(_("Accept compound words|#w")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_input_enc = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 130, 260, 30, 30, idex(_("Use input encoding|#i")));
    fl_set_button_shortcut(obj, scex(_("Use input encoding|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_inputs_misc::~FD_form_inputs_misc()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_inputs_misc * FormPreferences::build_inputs_misc()
{
  FL_OBJECT *obj;
  FD_form_inputs_misc *fdui = new FD_form_inputs_misc;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  fdui->input_date_format = obj = fl_add_input(FL_NORMAL_INPUT, 120, 70, 285, 30, idex(_("date format|#f")));
    fl_set_button_shortcut(obj, scex(_("date format|#f")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_language::~FD_form_language()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_language * FormPreferences::build_language()
{
  FL_OBJECT *obj;
  FD_form_language *fdui = new FD_form_language;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  fdui->input_package = obj = fl_add_input(FL_NORMAL_INPUT, 131, 18, 300, 30, idex(_("Package|#P")));
    fl_set_button_shortcut(obj, scex(_("Package|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_default_lang = obj = fl_add_choice(FL_NORMAL_CHOICE, 231, 56, 200, 30, idex(_("Default language|#l")));
    fl_set_button_shortcut(obj, scex(_("Default language|#l")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_use_kbmap = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 158, 113, 30, 30, idex(_("Keyboard map|#K")));
    fl_set_button_shortcut(obj, scex(_("Keyboard map|#K")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_kbmap_1 = obj = fl_add_choice(FL_NORMAL_CHOICE, 231, 96, 200, 30, idex(_("1st|#1")));
    fl_set_button_shortcut(obj, scex(_("1st|#1")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_kbmap_2 = obj = fl_add_choice(FL_NORMAL_CHOICE, 231, 128, 200, 30, idex(_("2nd|#2")));
    fl_set_button_shortcut(obj, scex(_("2nd|#2")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_rtl_support = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 154, 161, 30, 30, idex(_("RtL support|#R")));
    fl_set_button_shortcut(obj, scex(_("RtL support|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_mark_foreign = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 154, 187, 30, 30, idex(_("Mark foreign|#M")));
    fl_set_button_shortcut(obj, scex(_("Mark foreign|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_auto_begin = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 362, 161, 30, 30, idex(_("Auto begin|#b")));
    fl_set_button_shortcut(obj, scex(_("Auto begin|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_auto_end = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 362, 187, 30, 30, idex(_("Auto finish|#f")));
    fl_set_button_shortcut(obj, scex(_("Auto finish|#f")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_command_begin = obj = fl_add_input(FL_NORMAL_INPUT, 131, 224, 300, 30, idex(_("Command start|#s")));
    fl_set_button_shortcut(obj, scex(_("Command start|#s")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_command_end = obj = fl_add_input(FL_NORMAL_INPUT, 131, 257, 300, 30, idex(_("Command end|#e")));
    fl_set_button_shortcut(obj, scex(_("Command end|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_colors::~FD_form_colors()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_colors * FormPreferences::build_colors()
{
  FL_OBJECT *obj;
  FD_form_colors *fdui = new FD_form_colors;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  fdui->browser_x11 = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 25, 150, 235, idex(_("X11 color names|#X")));
    fl_set_button_shortcut(obj, scex(_("X11 color names|#X")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_name = obj = fl_add_input(FL_NORMAL_INPUT, 10, 285, 185, 30, idex(_("Name database|#N")));
    fl_set_button_shortcut(obj, scex(_("Name database|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 285, 90, 30, idex(_("Browse|#B")));
    fl_set_button_shortcut(obj, scex(_("Browse|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_color = obj = fl_add_button(FL_NORMAL_BUTTON, 180, 230, 90, 30, "");
    fl_set_object_boxtype(obj, FL_BORDER_BOX);
    fl_set_object_color(obj, FL_WHITE, FL_COL1);
  fdui->valslider_red = obj = fl_add_valslider(FL_VERT_FILL_SLIDER, 180, 25, 30, 200, idex(_("R|#R")));
    fl_set_button_shortcut(obj, scex(_("R|#R")), 1);
    fl_set_object_color(obj, FL_COL1, FL_RED);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  fdui->valslider_green = obj = fl_add_valslider(FL_VERT_FILL_SLIDER, 210, 25, 30, 200, idex(_("G|#G")));
    fl_set_button_shortcut(obj, scex(_("G|#G")), 1);
    fl_set_object_color(obj, FL_COL1, FL_GREEN);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  fdui->valslider_blue = obj = fl_add_valslider(FL_VERT_FILL_SLIDER, 240, 25, 30, 200, idex(_("B|#B")));
    fl_set_button_shortcut(obj, scex(_("B|#B")), 1);
    fl_set_object_color(obj, FL_COL1, FL_BLUE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  fdui->browser_lyx_objs = obj = fl_add_browser(FL_HOLD_BROWSER, 290, 25, 150, 251, idex(_("LyX objects|#L")));
    fl_set_button_shortcut(obj, scex(_("LyX objects|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_modify = obj = fl_add_button(FL_NORMAL_BUTTON, 320, 285, 90, 30, idex(_("Modify|#M")));
    fl_set_button_shortcut(obj, scex(_("Modify|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_converters::~FD_form_converters()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_converters * FormPreferences::build_converters()
{
  FL_OBJECT *obj;
  FD_form_converters *fdui = new FD_form_converters;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  fdui->browser_all = obj = fl_add_browser(FL_HOLD_BROWSER, 30, 30, 160, 270, idex(_("All converters|#A")));
    fl_set_button_shortcut(obj, scex(_("All converters|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_delete = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 270, 90, 30, idex(_("Delete|#D")));
    fl_set_button_shortcut(obj, scex(_("Delete|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_add = obj = fl_add_button(FL_NORMAL_BUTTON, 240, 270, 90, 30, idex(_("Add|#A")));
    fl_set_button_shortcut(obj, scex(_("Add|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_converter = obj = fl_add_input(FL_NORMAL_INPUT, 280, 110, 150, 30, idex(_("Converter|#C")));
    fl_set_button_shortcut(obj, scex(_("Converter|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_from = obj = fl_add_choice(FL_NORMAL_CHOICE, 280, 30, 150, 30, idex(_("From|#F")));
    fl_set_button_shortcut(obj, scex(_("From|#F")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_to = obj = fl_add_choice(FL_NORMAL_CHOICE, 280, 70, 150, 30, idex(_("To|#T")));
    fl_set_button_shortcut(obj, scex(_("To|#T")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_flags = obj = fl_add_input(FL_NORMAL_INPUT, 280, 150, 150, 30, idex(_("Flags|#F")));
    fl_set_button_shortcut(obj, scex(_("Flags|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_formats::~FD_form_formats()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_formats * FormPreferences::build_formats()
{
  FL_OBJECT *obj;
  FD_form_formats *fdui = new FD_form_formats;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  fdui->browser_all = obj = fl_add_browser(FL_HOLD_BROWSER, 30, 30, 160, 270, idex(_("All formats|#A")));
    fl_set_button_shortcut(obj, scex(_("All formats|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_format = obj = fl_add_input(FL_NORMAL_INPUT, 280, 30, 150, 30, idex(_("Format|#F")));
    fl_set_button_shortcut(obj, scex(_("Format|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_gui_name = obj = fl_add_input(FL_NORMAL_INPUT, 280, 70, 150, 30, idex(_("GUI name|#G")));
    fl_set_button_shortcut(obj, scex(_("GUI name|#G")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_delete = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 270, 90, 30, idex(_("Delete|#D")));
    fl_set_button_shortcut(obj, scex(_("Delete|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_add = obj = fl_add_button(FL_NORMAL_BUTTON, 240, 270, 90, 30, idex(_("Add|#A")));
    fl_set_button_shortcut(obj, scex(_("Add|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_extension = obj = fl_add_input(FL_NORMAL_INPUT, 280, 150, 150, 30, idex(_("Extension|#E")));
    fl_set_button_shortcut(obj, scex(_("Extension|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_viewer = obj = fl_add_input(FL_NORMAL_INPUT, 280, 190, 150, 30, idex(_("Viewer|#V")));
    fl_set_button_shortcut(obj, scex(_("Viewer|#V")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_shrtcut = obj = fl_add_input(FL_NORMAL_INPUT, 280, 110, 150, 30, idex(_("Shortcut|#S")));
    fl_set_button_shortcut(obj, scex(_("Shortcut|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_lnf_misc::~FD_form_lnf_misc()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_lnf_misc * FormPreferences::build_lnf_misc()
{
  FL_OBJECT *obj;
  FD_form_lnf_misc *fdui = new FD_form_lnf_misc;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 350);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 350, "");
  fdui->check_banner = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 15, 30, 30, idex(_("Show banner|#S")));
    fl_set_button_shortcut(obj, scex(_("Show banner|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_auto_region_delete = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 45, 30, 30, idex(_("Auto region delete|#A")));
    fl_set_button_shortcut(obj, scex(_("Auto region delete|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_exit_confirm = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 75, 30, 30, idex(_("Exit confirmation|#E")));
    fl_set_button_shortcut(obj, scex(_("Exit confirmation|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_display_shrtcuts = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 105, 30, 30, _("Display keyboard shortcuts"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->counter_autosave = obj = fl_add_counter(FL_SIMPLE_COUNTER, 320, 255, 115, 30, _("Autosave interval"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 1200);
    fl_set_counter_value(obj, 300);
    fl_set_counter_step(obj, 1, 1);
  fdui->check_ask_new_file = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 135, 30, 30, idex(_("File->New asks for name|#N")));
    fl_set_button_shortcut(obj, scex(_("File->New asks for name|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_cursor_follows_scrollbar = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 165, 30, 30, idex(_("Cursor follows scrollbar|#C")));
    fl_set_button_shortcut(obj, scex(_("Cursor follows scrollbar|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->counter_wm_jump = obj = fl_add_counter(FL_SIMPLE_COUNTER, 320, 225, 115, 30, _("Wheel mouse jump"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 250);
    fl_set_counter_value(obj, 100);
    fl_set_counter_step(obj, 1, 1);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_interface::~FD_form_interface()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_interface * FormPreferences::build_interface()
{
  FL_OBJECT *obj;
  FD_form_interface *fdui = new FD_form_interface;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 350);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 350, "");
  fdui->input_popup_font = obj = fl_add_input(FL_NORMAL_INPUT, 230, 30, 200, 30, _("Popup Font"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_menu_font = obj = fl_add_input(FL_NORMAL_INPUT, 230, 60, 200, 30, _("Menu Font"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_popup_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 230, 90, 200, 30, _("Popup Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_bind_file = obj = fl_add_input(FL_NORMAL_INPUT, 160, 178, 190, 30, idex(_("Bind file|#B")));
    fl_set_button_shortcut(obj, scex(_("Bind file|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_bind_file_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 178, 80, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_ui_file = obj = fl_add_input(FL_NORMAL_INPUT, 160, 148, 190, 30, idex(_("User Interface file|#U")));
    fl_set_button_shortcut(obj, scex(_("User Interface file|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ui_file_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 148, 80, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_override_x_dead_keys = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 24, 230, 30, 30, idex(_("Override X-Windows dead-keys|#O")));
    fl_set_button_shortcut(obj, scex(_("Override X-Windows dead-keys|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_printer::~FD_form_printer()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_printer * FormPreferences::build_printer()
{
  FL_OBJECT *obj;
  FD_form_printer *fdui = new FD_form_printer;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 350);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 350, "");
  fdui->input_command = obj = fl_add_input(FL_NORMAL_INPUT, 130, 75, 80, 30, _("command"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_page_range = obj = fl_add_input(FL_NORMAL_INPUT, 130, 105, 80, 30, _("page range"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_copies = obj = fl_add_input(FL_NORMAL_INPUT, 130, 135, 80, 30, _("copies"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_reverse = obj = fl_add_input(FL_NORMAL_INPUT, 130, 165, 80, 30, _("reverse"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_to_printer = obj = fl_add_input(FL_NORMAL_INPUT, 130, 195, 80, 30, _("to printer"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_file_extension = obj = fl_add_input(FL_NORMAL_INPUT, 130, 225, 80, 30, _("file extension"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_spool_command = obj = fl_add_input(FL_NORMAL_INPUT, 130, 255, 80, 30, _("spool command"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_paper_type = obj = fl_add_input(FL_NORMAL_INPUT, 130, 285, 80, 30, _("paper type"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_even_pages = obj = fl_add_input(FL_NORMAL_INPUT, 360, 75, 80, 30, _("even pages"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_odd_pages = obj = fl_add_input(FL_NORMAL_INPUT, 360, 105, 80, 30, _("odd pages"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_collated = obj = fl_add_input(FL_NORMAL_INPUT, 360, 135, 80, 30, _("collated"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_landscape = obj = fl_add_input(FL_NORMAL_INPUT, 360, 165, 80, 30, _("landscape"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_to_file = obj = fl_add_input(FL_NORMAL_INPUT, 360, 195, 80, 30, _("to file"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_extra_options = obj = fl_add_input(FL_NORMAL_INPUT, 360, 225, 80, 30, _("extra options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_spool_prefix = obj = fl_add_input(FL_NORMAL_INPUT, 360, 255, 80, 30, _("spool printer prefix"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_paper_size = obj = fl_add_input(FL_NORMAL_INPUT, 360, 285, 80, 30, _("paper size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_name = obj = fl_add_input(FL_NORMAL_INPUT, 130, 10, 80, 30, _("name"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_adapt_output = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 240, 10, 30, 30, _("adapt output"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 0, 60, 452, 260, _("Printer Command and Flags"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_paths::~FD_form_paths()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_paths * FormPreferences::build_paths()
{
  FL_OBJECT *obj;
  FD_form_paths *fdui = new FD_form_paths;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 350);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 350, "");
  fdui->input_default_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 10, 170, 30, idex(_("Default path|#p")));
    fl_set_button_shortcut(obj, scex(_("Default path|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_document_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 10, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->counter_lastfiles = obj = fl_add_counter(FL_SIMPLE_COUNTER, 170, 130, 90, 30, idex(_("Last file count|#L")));
    fl_set_button_shortcut(obj, scex(_("Last file count|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 9);
    fl_set_counter_value(obj, 4);
    fl_set_counter_step(obj, 1, 1);
  fdui->input_template_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 40, 170, 30, idex(_("Template path|#T")));
    fl_set_button_shortcut(obj, scex(_("Template path|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_template_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 40, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_last_files = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 134, 100, 30, 30, idex(_("Check last files|#C")));
    fl_set_button_shortcut(obj, scex(_("Check last files|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->button_temp_dir_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 70, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_lastfiles = obj = fl_add_input(FL_NORMAL_INPUT, 170, 100, 170, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_lastfiles_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 100, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_make_backups = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 134, 170, 30, 30, idex(_("Backup path|#B")));
    fl_set_button_shortcut(obj, scex(_("Backup path|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_backup_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 170, 170, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_backup_path_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 170, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_serverpipe = obj = fl_add_input(FL_NORMAL_INPUT, 169, 217, 170, 30, idex(_("LyXServer pipe|#S")));
    fl_set_button_shortcut(obj, scex(_("LyXServer pipe|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_serverpipe_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 339, 217, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_temp_dir = obj = fl_add_input(FL_NORMAL_INPUT, 170, 70, 170, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_use_temp_dir = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 134, 70, 30, 30, idex(_("Temp dir|#d")));
    fl_set_button_shortcut(obj, scex(_("Temp dir|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

