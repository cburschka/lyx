// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
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
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 5, 390, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 170, 390, 90, 30, _("Save"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 390, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 370, 390, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
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
  fdui->input_roman = obj = fl_add_input(FL_NORMAL_INPUT, 145, 25, 140, 30, _("Roman"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_sans = obj = fl_add_input(FL_NORMAL_INPUT, 145, 60, 140, 30, _("Sans Serif"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_typewriter = obj = fl_add_input(FL_NORMAL_INPUT, 145, 95, 140, 30, _("Typewriter"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_screen_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 145, 130, 140, 30, _("Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_scalable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 190, 34, 41, _("Use scalable fonts"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    // xgettext:no-c-format
    char const * const dummy = N_("Zoom %|#Z");
    fdui->counter_zoom = obj = fl_add_counter(FL_NORMAL_COUNTER, 185, 245, 95, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 999);
    fl_set_counter_value(obj, 150);
    fl_set_counter_step(obj, 1, 1);
  {
    char const * const dummy = N_("Screen DPI|#D");
    fdui->counter_dpi = obj = fl_add_counter(FL_NORMAL_COUNTER, 185, 280, 95, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 999);
    fl_set_counter_value(obj, 150);
    fl_set_counter_step(obj, 1, 1);
  fdui->input_tiny = obj = fl_add_input(FL_FLOAT_INPUT, 370, 20, 70, 30, _("tiny"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_script = obj = fl_add_input(FL_FLOAT_INPUT, 370, 50, 70, 30, _("smallest"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_footnote = obj = fl_add_input(FL_FLOAT_INPUT, 370, 80, 70, 30, _("smaller"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_small = obj = fl_add_input(FL_FLOAT_INPUT, 370, 110, 70, 30, _("small"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_normal = obj = fl_add_input(FL_FLOAT_INPUT, 370, 140, 70, 30, _("normal"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_large = obj = fl_add_input(FL_FLOAT_INPUT, 370, 170, 70, 30, _("large"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_larger = obj = fl_add_input(FL_FLOAT_INPUT, 370, 200, 70, 30, _("larger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_largest = obj = fl_add_input(FL_FLOAT_INPUT, 370, 230, 70, 30, _("largest"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_huge = obj = fl_add_input(FL_FLOAT_INPUT, 370, 260, 70, 30, _("huge"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_huger = obj = fl_add_input(FL_FLOAT_INPUT, 370, 290, 70, 30, _("huger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 15, 285, 155, _("Fonts used"));
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 305, 15, 140, 310, _("Size"));
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 180, 285, 145, _("Scale & Resolution"));
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
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_menu_font = obj = fl_add_input(FL_NORMAL_INPUT, 230, 60, 200, 30, _("Menu Font"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_popup_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 230, 90, 200, 30, _("Popup Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("User Interface file|#U");
    fdui->input_ui_file = obj = fl_add_input(FL_NORMAL_INPUT, 160, 160, 170, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#r");
    fdui->button_ui_file_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 335, 160, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Bind file|#B");
    fdui->input_bind_file = obj = fl_add_input(FL_NORMAL_INPUT, 160, 195, 170, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#w");
    fdui->button_bind_file_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 335, 195, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Override X Window dead-keys|#O");
    fdui->check_override_x_dead_keys = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 95, 265, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 20, 425, 110, _("Fonts & Encoding"));
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 150, 425, 85, _("Layout & Bindings"));
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 255, 425, 45, _("Dead Keys"));
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
  {
    char const * const dummy = N_("LyX objects|#L");
    fdui->browser_lyx_objs = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 25, 195, 290, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("H|#H");
    fdui->dial_hue = obj = fl_add_dial(FL_NORMAL_DIAL, 276, 23, 100, 100, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_CENTER);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("S|#S");
    fdui->slider_saturation = obj = fl_add_slider(FL_HOR_NICE_SLIDER, 251, 135, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  {
    char const * const dummy = N_("V|#V");
    fdui->slider_value = obj = fl_add_slider(FL_HOR_NICE_SLIDER, 251, 175, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  {
    char const * const dummy = N_("R|#R");
    fdui->slider_red = obj = fl_add_slider(FL_VERT_FILL_SLIDER, 250, 25, 40, 180, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_RED);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  {
    char const * const dummy = N_("G|#G");
    fdui->slider_green = obj = fl_add_slider(FL_VERT_FILL_SLIDER, 305, 25, 40, 180, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_GREEN);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_size(obj, 0.15);
  {
    char const * const dummy = N_("B|#B");
    fdui->slider_blue = obj = fl_add_slider(FL_VERT_FILL_SLIDER, 360, 25, 40, 180, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_BLUE);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_slider_size(obj, 0.15);

  fdui->group_radio_printto = fl_bgn_group();
  fdui->radio_hsv = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 250, 220, 30, 30, _("HSV"));
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->radio_rgb = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 335, 220, 30, 30, _("RGB"));
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  fl_end_group();

  fdui->text_color_values = obj = fl_add_text(FL_NORMAL_TEXT, 250, 250, 150, 30, "");
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->button_color = obj = fl_add_button(FL_NORMAL_BUTTON, 250, 285, 60, 30, "");
    fl_set_object_boxtype(obj, FL_BORDER_BOX);
    fl_set_object_color(obj, FL_WHITE, FL_COL1);
  {
    char const * const dummy = N_("Modify|#M");
    fdui->button_modify = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 285, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
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
  {
    char const * const dummy = N_("Show banner|#S");
    fdui->check_banner = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 15, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Auto region delete|#A");
    fdui->check_auto_region_delete = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 45, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Exit confirmation|#E");
    fdui->check_exit_confirm = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 75, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_display_shrtcuts = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 105, 30, 30, _("Display keyboard shortcuts"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("File->New asks for name|#N");
    fdui->check_ask_new_file = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 135, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Cursor follows scrollbar|#C");
    fdui->check_cursor_follows_scrollbar = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 165, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Dialogs iconify with main window|#D");
    fdui->check_dialogs_iconify_with_main = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 195, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  fdui->counter_wm_jump = obj = fl_add_counter(FL_NORMAL_COUNTER, 20, 235, 115, 30, _("Wheel mouse jump"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 250);
    fl_set_counter_value(obj, 100);
    fl_set_counter_step(obj, 1, 1);
  fdui->counter_autosave = obj = fl_add_counter(FL_NORMAL_COUNTER, 20, 270, 115, 30, _("Autosave interval"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 1200);
    fl_set_counter_value(obj, 300);
    fl_set_counter_step(obj, 1, 1);

  fdui->radio_button_group_display = fl_bgn_group();
  {
    char const * const dummy = N_("in Monochrome|#M");
    fdui->radio_display_monochrome = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 285, 35, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("in Grayscale|#G");
    fdui->radio_display_grayscale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 285, 65, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("in Color|#C");
    fdui->radio_display_color = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 285, 95, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Don't display|#D");
    fdui->radio_no_display = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 285, 125, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_group();

  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 275, 15, 160, 210, _("Display Graphics"));
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 15, 15, 255, 210, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 15, 230, 420, 75, "");
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_spelloptions::~FD_form_spelloptions()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_spelloptions * FormPreferences::build_spelloptions()
{
  FL_OBJECT *obj;
  FD_form_spelloptions *fdui = new FD_form_spelloptions;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 360, "");
  {
    char const * const dummy = N_("Spell command|#S");
    fdui->choice_spell_command = obj = fl_add_choice(FL_NORMAL_CHOICE, 240, 30, 171, 34, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use alternative language|#a");
    fdui->check_alt_lang = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 208, 100, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_alt_lang = obj = fl_add_input(FL_NORMAL_INPUT, 239, 100, 170, 30, "");
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use escape characters|#e");
    fdui->check_escape_chars = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 208, 130, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_escape_chars = obj = fl_add_input(FL_NORMAL_INPUT, 239, 130, 170, 30, "");
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use personal dictionary|#d");
    fdui->check_personal_dict = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 208, 160, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_personal_dict = obj = fl_add_input(FL_NORMAL_INPUT, 239, 160, 170, 30, "");
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#B");
    fdui->button_personal_dict = obj = fl_add_button(FL_NORMAL_BUTTON, 240, 195, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Accept compound words|#w");
    fdui->check_compound_words = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 155, 250, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use input encoding|#i");
    fdui->check_input_enc = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 155, 275, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 245, 425, 60, _("Advanced Options"));
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 20, 425, 55, _("Interface"));
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 90, 425, 140, _("Language Options"));
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
  {
    char const * const dummy = N_("Package|#P");
    fdui->input_package = obj = fl_add_input(FL_NORMAL_INPUT, 131, 18, 300, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Default language|#l");
    fdui->choice_default_lang = obj = fl_add_choice(FL_NORMAL_CHOICE, 231, 56, 200, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Keyboard\nmap|#K");
    fdui->check_use_kbmap = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 78, 113, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("1st|#1");
    fdui->input_kbmap1 = obj = fl_add_input(FL_NORMAL_INPUT, 145, 100, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("2nd|#2");
    fdui->input_kbmap2 = obj = fl_add_input(FL_NORMAL_INPUT, 145, 135, 190, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#o");
    fdui->button_kbmap1_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 100, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#w");
    fdui->button_kbmap2_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 135, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("RtL support|#R");
    fdui->check_rtl_support = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Auto begin|#b");
    fdui->check_auto_begin = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 265, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Use babel|#U");
    fdui->check_use_babel = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 390, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Mark foreign|#M");
    fdui->check_mark_foreign = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 205, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Auto finish|#f");
    fdui->check_auto_end = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 265, 205, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Global|#G");
    fdui->check_global_options = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 390, 205, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Command start|#s");
    fdui->input_command_begin = obj = fl_add_input(FL_NORMAL_INPUT, 130, 245, 300, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Command end|#e");
    fdui->input_command_end = obj = fl_add_input(FL_NORMAL_INPUT, 130, 280, 300, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 5, 175, 440, 60, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 5, 240, 440, 80, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 5, 95, 440, 75, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 5, 10, 440, 80, "");
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
  {
    char const * const dummy = N_("All formats|#A");
    fdui->browser_all = obj = fl_add_browser(FL_HOLD_BROWSER, 30, 30, 160, 270, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Format|#F");
    fdui->input_format = obj = fl_add_input(FL_NORMAL_INPUT, 280, 30, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("GUI name|#G");
    fdui->input_gui_name = obj = fl_add_input(FL_NORMAL_INPUT, 280, 70, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Shortcut|#S");
    fdui->input_shrtcut = obj = fl_add_input(FL_NORMAL_INPUT, 280, 110, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Extension|#E");
    fdui->input_extension = obj = fl_add_input(FL_NORMAL_INPUT, 280, 150, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Viewer|#V");
    fdui->input_viewer = obj = fl_add_input(FL_NORMAL_INPUT, 280, 190, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Add|#A");
    fdui->button_add = obj = fl_add_button(FL_NORMAL_BUTTON, 240, 270, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Delete|#D");
    fdui->button_delete = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 270, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
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
  {
    char const * const dummy = N_("All converters|#A");
    fdui->browser_all = obj = fl_add_browser(FL_HOLD_BROWSER, 30, 30, 160, 270, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("From|#F");
    fdui->choice_from = obj = fl_add_choice(FL_NORMAL_CHOICE, 280, 30, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("To|#T");
    fdui->choice_to = obj = fl_add_choice(FL_NORMAL_CHOICE, 280, 70, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Converter|#C");
    fdui->input_converter = obj = fl_add_input(FL_NORMAL_INPUT, 280, 110, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Flags|#F");
    fdui->input_flags = obj = fl_add_input(FL_NORMAL_INPUT, 280, 150, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Add|#A");
    fdui->button_add = obj = fl_add_button(FL_NORMAL_BUTTON, 240, 270, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Delete|#D");
    fdui->button_delete = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 270, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
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
  {
    char const * const dummy = N_("Default path|#p");
    fdui->input_default_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 10, 170, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_default_path_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 345, 10, 100, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Template path|#T");
    fdui->input_template_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 45, 170, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_template_path_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 345, 45, 100, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Temp dir|#d");
    fdui->check_use_temp_dir = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 80, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_temp_dir = obj = fl_add_input(FL_NORMAL_INPUT, 170, 80, 170, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_temp_dir_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 345, 80, 100, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Check last files|#C");
    fdui->check_last_files = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 115, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_lastfiles = obj = fl_add_input(FL_NORMAL_INPUT, 170, 115, 170, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_lastfiles_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 345, 115, 100, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Last file count|#L");
    fdui->counter_lastfiles = obj = fl_add_counter(FL_SIMPLE_COUNTER, 170, 150, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 9);
    fl_set_counter_value(obj, 4);
    fl_set_counter_step(obj, 1, 1);
  {
    char const * const dummy = N_("Backup path|#B");
    fdui->check_make_backups = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 140, 205, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_backup_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 205, 170, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_backup_path_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 345, 205, 100, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("LyXServer pipe|#S");
    fdui->input_serverpipe = obj = fl_add_input(FL_NORMAL_INPUT, 170, 245, 170, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->button_serverpipe_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 345, 245, 100, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
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
  {
    char const * const dummy = N_("date format|#f");
    fdui->input_date_format = obj = fl_add_input(FL_NORMAL_INPUT, 120, 70, 285, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
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
  fdui->input_name = obj = fl_add_input(FL_NORMAL_INPUT, 130, 10, 80, 30, _("name"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->check_adapt_output = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 240, 10, 30, 30, _("adapt output"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 60, 435, 260, _("Printer Command and Flags"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_command = obj = fl_add_input(FL_NORMAL_INPUT, 130, 75, 80, 30, _("command"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_page_range = obj = fl_add_input(FL_NORMAL_INPUT, 130, 105, 80, 30, _("page range"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_copies = obj = fl_add_input(FL_NORMAL_INPUT, 130, 135, 80, 30, _("copies"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_reverse = obj = fl_add_input(FL_NORMAL_INPUT, 130, 165, 80, 30, _("reverse"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_to_printer = obj = fl_add_input(FL_NORMAL_INPUT, 130, 195, 80, 30, _("to printer"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_file_extension = obj = fl_add_input(FL_NORMAL_INPUT, 130, 225, 80, 30, _("file extension"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_spool_command = obj = fl_add_input(FL_NORMAL_INPUT, 130, 255, 80, 30, _("spool command"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_paper_type = obj = fl_add_input(FL_NORMAL_INPUT, 130, 285, 80, 30, _("paper type"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_even_pages = obj = fl_add_input(FL_NORMAL_INPUT, 360, 75, 80, 30, _("even pages"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_odd_pages = obj = fl_add_input(FL_NORMAL_INPUT, 360, 105, 80, 30, _("odd pages"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_collated = obj = fl_add_input(FL_NORMAL_INPUT, 360, 135, 80, 30, _("collated"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_landscape = obj = fl_add_input(FL_NORMAL_INPUT, 360, 165, 80, 30, _("landscape"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_to_file = obj = fl_add_input(FL_NORMAL_INPUT, 360, 195, 80, 30, _("to file"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_extra_options = obj = fl_add_input(FL_NORMAL_INPUT, 360, 225, 80, 30, _("extra options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_spool_prefix = obj = fl_add_input(FL_NORMAL_INPUT, 360, 255, 80, 30, _("spool printer prefix"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_paper_size = obj = fl_add_input(FL_NORMAL_INPUT, 360, 285, 80, 30, _("paper size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 5, 435, 45, "");
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
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  {
    char const * const dummy = N_("Ascii line length|#A");
    fdui->counter_line_len = obj = fl_add_counter(FL_NORMAL_COUNTER, 330, 25, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 120);
    fl_set_counter_value(obj, 75);
    fl_set_counter_step(obj, 1, 1);
  {
    char const * const dummy = N_("TeX encoding|#T");
    fdui->input_tex_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 230, 65, 200, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("Default paper size|#p");
    fdui->choice_default_papersize = obj = fl_add_choice(FL_NORMAL_CHOICE, 230, 105, 199, 29, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 190, 435, 135, _("Outside code interaction"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("ascii roff|#r");
    fdui->input_ascii_roff = obj = fl_add_input(FL_NORMAL_INPUT, 145, 205, 285, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  {
    char const * const dummy = N_("checktex|#c");
    fdui->input_checktex = obj = fl_add_input(FL_NORMAL_INPUT, 145, 245, 285, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fdui->input_paperoption = obj = fl_add_input(FL_NORMAL_INPUT, 145, 285, 285, 30, _("DVI paper option"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 10, 435, 165, "");
  {
    char const * const dummy = N_("Autoreset Class Options on change|#u");
    fdui->check_autoreset_classopt = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 140, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

