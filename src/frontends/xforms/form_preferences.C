// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_preferences.h"
#include "FormPreferences.h"

FD_form_lnf_general::~FD_form_lnf_general()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_lnf_general * FormPreferences::build_lnf_general()
{
  FL_OBJECT *obj;
  FD_form_lnf_general *fdui = new FD_form_lnf_general;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 320);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 320, "");
  fdui->check_banner = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 15, 310, 30, idex(_("Show banner|#S")));
    fl_set_button_shortcut(obj, scex(_("Show banner|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_auto_region_delete = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 45, 310, 30, idex(_("Auto region delete|#A")));
    fl_set_button_shortcut(obj, scex(_("Auto region delete|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_exit_confirm = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 75, 310, 30, idex(_("Exit confirmation|#E")));
    fl_set_button_shortcut(obj, scex(_("Exit confirmation|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_display_shortcuts = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 105, 310, 30, _("Display keyboard shortcuts"));
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
  fdui->check_ask_new_file = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 135, 310, 30, idex(_("File->New asks for name|#N")));
    fl_set_button_shortcut(obj, scex(_("File->New asks for name|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_cursor_follows_scrollbar = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 165, 310, 30, idex(_("Cursor follows scrollbar|#C")));
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

FD_form_screen_fonts::~FD_form_screen_fonts()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_screen_fonts * FormPreferences::build_screen_fonts()
{
  FL_OBJECT *obj;
  FD_form_screen_fonts *fdui = new FD_form_screen_fonts;

  fdui->form = fl_bgn_form(FL_NO_BOX, 455, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 455, 345, "");
  fdui->input_roman = obj = fl_add_input(FL_NORMAL_INPUT, 210, 5, 200, 30, _("Roman"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_sans = obj = fl_add_input(FL_NORMAL_INPUT, 210, 35, 200, 30, _("Sans Serif"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_typewriter = obj = fl_add_input(FL_NORMAL_INPUT, 210, 65, 200, 30, _("Typewriter"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->counter_zoom = obj = fl_add_counter(FL_SIMPLE_COUNTER, 210, 125, 200, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 999);
    fl_set_counter_value(obj, 150);
    fl_set_counter_step(obj, 1, 1);
  fdui->check_scalable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 50, 125, 160, 30, _("Allow scaling"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_screen_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 210, 95, 200, 30, _("Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_tiny = obj = fl_add_input(FL_FLOAT_INPUT, 140, 165, 70, 30, _("tiny"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_script = obj = fl_add_input(FL_FLOAT_INPUT, 340, 165, 70, 30, _("script"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_footnote = obj = fl_add_input(FL_FLOAT_INPUT, 140, 195, 70, 30, _("footnote"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_small = obj = fl_add_input(FL_FLOAT_INPUT, 340, 195, 70, 30, _("small"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_large = obj = fl_add_input(FL_FLOAT_INPUT, 340, 225, 70, 30, _("large"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_larger = obj = fl_add_input(FL_FLOAT_INPUT, 140, 255, 70, 30, _("larger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_largest = obj = fl_add_input(FL_FLOAT_INPUT, 340, 255, 70, 30, _("largest"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_normal = obj = fl_add_input(FL_FLOAT_INPUT, 140, 225, 70, 30, _("normal"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_huge = obj = fl_add_input(FL_FLOAT_INPUT, 140, 285, 70, 30, _("huge"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_huger = obj = fl_add_input(FL_FLOAT_INPUT, 340, 285, 70, 30, _("huger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_interface::~FD_form_interface()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_interface * FormPreferences::build_interface()
{
  FL_OBJECT *obj;
  FD_form_interface *fdui = new FD_form_interface;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 320);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 320, "");
  fdui->input_popup_font = obj = fl_add_input(FL_NORMAL_INPUT, 230, 50, 200, 30, _("Popup Font"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_menu_font = obj = fl_add_input(FL_NORMAL_INPUT, 230, 80, 200, 30, _("Menu Font"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_popup_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 230, 110, 200, 30, _("Popup Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_bind_file = obj = fl_add_input(FL_NORMAL_INPUT, 160, 210, 190, 30, idex(_("Bind file|#B")));
    fl_set_button_shortcut(obj, scex(_("Bind file|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_bind_file_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 210, 80, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_ui_file = obj = fl_add_input(FL_NORMAL_INPUT, 160, 180, 190, 30, idex(_("User Interface file|#U")));
    fl_set_button_shortcut(obj, scex(_("User Interface file|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_ui_file_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 180, 80, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_printer::~FD_form_printer()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_printer * FormPreferences::build_printer()
{
  FL_OBJECT *obj;
  FD_form_printer *fdui = new FD_form_printer;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 320);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 320, "");
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
  fdui->check_adapt_output = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 240, 10, 120, 30, _("adapt output"));
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
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_paths * FormPreferences::build_paths()
{
  FL_OBJECT *obj;
  FD_form_paths *fdui = new FD_form_paths;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 320);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 320, "");
  fdui->input_default_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 10, 170, 30, _("Default path"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_document_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 10, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->counter_lastfiles = obj = fl_add_counter(FL_SIMPLE_COUNTER, 170, 130, 90, 30, _("Last file count"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 9);
    fl_set_counter_value(obj, 4);
    fl_set_counter_step(obj, 1, 1);
  fdui->input_template_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 40, 170, 30, _("Template path"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_template_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 40, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_last_files = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 290, 130, 140, 30, _("Check last files"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_temp_dir = obj = fl_add_input(FL_NORMAL_INPUT, 170, 70, 170, 30, _("Temp dir"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_temp_dir_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 70, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_lastfiles = obj = fl_add_input(FL_NORMAL_INPUT, 170, 100, 170, 30, _("Lastfiles"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_lastfiles_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 100, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_use_temp_dir = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 35, 70, 50, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->check_make_backups = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 30, 170, 40, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  fdui->input_backup_path = obj = fl_add_input(FL_NORMAL_INPUT, 170, 170, 170, 30, _("Backup path"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->button_backup_path_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 170, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_preferences::~FD_form_preferences()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_preferences * FormPreferences::build_preferences()
{
  FL_OBJECT *obj;
  FD_form_preferences *fdui = new FD_form_preferences;

  fdui->form = fl_bgn_form(FL_NO_BOX, 470, 475);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 470, 475, "");
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 380, 90, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 370, 380, 90, 30, idex(_("Cancel|^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 170, 380, 90, 30, _("Save"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->tabfolder_prefs = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 5, 10, 455, 365, "");
    fl_set_object_boxtype(obj, FL_FLAT_BOX);
  fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 5, 380, 90, 30, idex(_("Restore|#R")));
    fl_set_button_shortcut(obj, scex(_("Restore|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 5, 420, 460, 55, "");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 0, 415, 470, 1, "");
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_outer_tab::~FD_form_outer_tab()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_outer_tab * FormPreferences::build_outer_tab()
{
  FL_OBJECT *obj;
  FD_form_outer_tab *fdui = new FD_form_outer_tab;

  fdui->form = fl_bgn_form(FL_NO_BOX, 455, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 455, 345, "");
  fdui->tabfolder_outer = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 0, 0, 455, 345, "");
    fl_set_object_boxtype(obj, FL_FLAT_BOX);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_outputs_general::~FD_form_outputs_general()
{
  if( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_outputs_general * FormPreferences::build_outputs_general()
{
  FL_OBJECT *obj;
  FD_form_outputs_general *fdui = new FD_form_outputs_general;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 320);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 450, 320, "");
  fdui->counter_line_len = obj = fl_add_counter(FL_SIMPLE_COUNTER, 295, 50, 120, 30, idex(_("Ascii line length|#A")));
    fl_set_button_shortcut(obj, scex(_("Ascii line length|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 120);
    fl_set_counter_value(obj, 75);
    fl_set_counter_step(obj, 1, 1);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

