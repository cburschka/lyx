// File modified by fdfix.sh for use by lyx (with xforms >= 0.86) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "FormPreferences.h"

FD_form_bind * FormPreferences::build_bind()
{
  FL_OBJECT *obj;
  FD_form_bind *fdui = new FD_form_bind;

  fdui->form_bind = fl_bgn_form(FL_NO_BOX, 380, 320);
  fdui->form_bind->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 380, 320, "");
  fdui->input_bind = obj = fl_add_input(FL_NORMAL_INPUT, 80, 80, 190, 30, _("Bind file"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_button(FL_NORMAL_BUTTON, 270, 80, 80, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form_bind->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_misc * FormPreferences::build_misc()
{
  FL_OBJECT *obj;
  FD_form_misc *fdui = new FD_form_misc;

  fdui->form_misc = fl_bgn_form(FL_NO_BOX, 380, 320);
  fdui->form_misc->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 380, 320, "");
  fdui->check_banner = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 30, 240, 30, _("Show banner"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->check_auto_region_delete = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 60, 240, 30, _("Auto region delete"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->check_exit_confirm = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 90, 240, 30, _("Exit confirmation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->check_display_shortcuts = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 120, 240, 30, _("Display keyboard shortcuts"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->counter_autosave = obj = fl_add_counter(FL_NORMAL_COUNTER, 160, 250, 170, 30, _("Autosave interval"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 1200);
    fl_set_counter_value(obj, 300);
    fl_set_counter_step(obj, 1, 1);
  fdui->counter_line_len = obj = fl_add_counter(FL_NORMAL_COUNTER, 160, 220, 170, 30, _("Ascii line length"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 120);
    fl_set_counter_value(obj, 75);
    fl_set_counter_step(obj, 1, 1);
  fl_end_form();

  fdui->form_misc->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_screen_fonts * FormPreferences::build_screen_fonts()
{
  FL_OBJECT *obj;
  FD_form_screen_fonts *fdui = new FD_form_screen_fonts;

  fdui->form_screen_fonts = fl_bgn_form(FL_NO_BOX, 380, 320);
  fdui->form_screen_fonts->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 380, 320, "");
  fdui->input_roman = obj = fl_add_input(FL_NORMAL_INPUT, 160, 10, 200, 30, _("Roman"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_sans = obj = fl_add_input(FL_NORMAL_INPUT, 160, 40, 200, 30, _("Sans Serif"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_typewriter = obj = fl_add_input(FL_NORMAL_INPUT, 160, 70, 200, 30, _("Typewriter"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->counter_zoom = obj = fl_add_counter(FL_NORMAL_COUNTER, 160, 130, 200, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 999);
    fl_set_counter_value(obj, 150);
    fl_set_counter_step(obj, 1, 1);
  fdui->check_scalable = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 0, 130, 160, 30, _("Allow scaling"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->input_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 160, 100, 200, 30, _("Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_tiny = obj = fl_add_input(FL_FLOAT_INPUT, 120, 170, 70, 30, _("tiny"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_script = obj = fl_add_input(FL_FLOAT_INPUT, 290, 170, 70, 30, _("script"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_footnote = obj = fl_add_input(FL_FLOAT_INPUT, 120, 200, 70, 30, _("footnote"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_small = obj = fl_add_input(FL_FLOAT_INPUT, 290, 200, 70, 30, _("small"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_large = obj = fl_add_input(FL_FLOAT_INPUT, 290, 230, 70, 30, _("large"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_larger = obj = fl_add_input(FL_FLOAT_INPUT, 120, 260, 70, 30, _("larger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_largest = obj = fl_add_input(FL_FLOAT_INPUT, 290, 260, 70, 30, _("largest"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_normal = obj = fl_add_input(FL_FLOAT_INPUT, 120, 230, 70, 30, _("normal"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_huge = obj = fl_add_input(FL_FLOAT_INPUT, 120, 290, 70, 30, _("huge"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_huger = obj = fl_add_input(FL_FLOAT_INPUT, 290, 290, 70, 30, _("huger"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form_screen_fonts->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_interface_fonts * FormPreferences::build_interface_fonts()
{
  FL_OBJECT *obj;
  FD_form_interface_fonts *fdui = new FD_form_interface_fonts;

  fdui->form_interface_fonts = fl_bgn_form(FL_NO_BOX, 380, 320);
  fdui->form_interface_fonts->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 380, 320, "");
  fdui->input_popup_font = obj = fl_add_input(FL_NORMAL_INPUT, 120, 50, 200, 30, _("Popup"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_menu_font = obj = fl_add_input(FL_NORMAL_INPUT, 120, 80, 200, 30, _("Menu"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_encoding = obj = fl_add_input(FL_NORMAL_INPUT, 120, 110, 200, 30, _("Encoding"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form_interface_fonts->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_printer * FormPreferences::build_printer()
{
  FL_OBJECT *obj;
  FD_form_printer *fdui = new FD_form_printer;

  fdui->form_printer = fl_bgn_form(FL_NO_BOX, 380, 320);
  fdui->form_printer->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 380, 320, "");
  fdui->input_command = obj = fl_add_input(FL_NORMAL_INPUT, 110, 60, 80, 30, _("command"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_page_range = obj = fl_add_input(FL_NORMAL_INPUT, 110, 90, 80, 30, _("page range"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_copies = obj = fl_add_input(FL_NORMAL_INPUT, 110, 120, 80, 30, _("copies"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_reverse = obj = fl_add_input(FL_NORMAL_INPUT, 110, 150, 80, 30, _("reverse"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_to_printer = obj = fl_add_input(FL_NORMAL_INPUT, 110, 180, 80, 30, _("to printer"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_file_extension = obj = fl_add_input(FL_NORMAL_INPUT, 110, 210, 80, 30, _("file extension"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_spool_command = obj = fl_add_input(FL_NORMAL_INPUT, 110, 240, 80, 30, _("spool command"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_paper_type = obj = fl_add_input(FL_NORMAL_INPUT, 110, 270, 80, 30, _("paper type"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_even_pages = obj = fl_add_input(FL_NORMAL_INPUT, 300, 60, 80, 30, _("even pages"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_odd_pages = obj = fl_add_input(FL_NORMAL_INPUT, 300, 90, 80, 30, _("odd pages"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_collated = obj = fl_add_input(FL_NORMAL_INPUT, 300, 120, 80, 30, _("collated"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_landscape = obj = fl_add_input(FL_NORMAL_INPUT, 300, 150, 80, 30, _("landscape"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_to_file = obj = fl_add_input(FL_NORMAL_INPUT, 300, 180, 80, 30, _("to file"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_extra_options = obj = fl_add_input(FL_NORMAL_INPUT, 300, 210, 80, 30, _("extra options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_spool_prefix = obj = fl_add_input(FL_NORMAL_INPUT, 300, 240, 80, 30, _("spool printer prefix"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_paper_size = obj = fl_add_input(FL_NORMAL_INPUT, 300, 270, 80, 30, _("paper size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_name = obj = fl_add_input(FL_NORMAL_INPUT, 110, 10, 80, 30, _("name"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_adapt_output = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 10, 120, 30, _("adapt output"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form_printer->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_paths * FormPreferences::build_paths()
{
  FL_OBJECT *obj;
  FD_form_paths *fdui = new FD_form_paths;

  fdui->form_paths = fl_bgn_form(FL_NO_BOX, 380, 430);
  fdui->form_paths->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 380, 430, "");
  fdui->input_default_path = obj = fl_add_input(FL_NORMAL_INPUT, 110, 10, 170, 30, _("Default path"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->button_document_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 280, 10, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->counter_lastfiles = obj = fl_add_counter(FL_NORMAL_COUNTER, 110, 130, 110, 30, _("Last file count"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, 0, 9);
    fl_set_counter_value(obj, 4);
    fl_set_counter_step(obj, 1, 1);
  fdui->input_template_path = obj = fl_add_input(FL_NORMAL_INPUT, 110, 40, 170, 30, _("Template path"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->button_template_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 280, 40, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_last_files = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 130, 140, 30, _("Check last files"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->input_temp_dir = obj = fl_add_input(FL_NORMAL_INPUT, 110, 70, 170, 30, _("Temp dir"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->button_temp_dir_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 280, 70, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->input_lastfiles = obj = fl_add_input(FL_NORMAL_INPUT, 110, 100, 170, 30, _("Lastfiles"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->button_lastfiles_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 280, 100, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->check_use_temp_dir = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 70, 50, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->check_make_backups = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 170, 40, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_button(obj, 1);
  fdui->input_backup_path = obj = fl_add_input(FL_NORMAL_INPUT, 110, 170, 170, 30, _("Backup path"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->button_backup_path_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 280, 170, 90, 30, _("Browse..."));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form_paths->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_preferences * FormPreferences::build_preferences()
{
  FL_OBJECT *obj;
  FD_form_preferences *fdui = new FD_form_preferences;

  fdui->form_preferences = fl_bgn_form(FL_NO_BOX, 380, 430);
  fdui->form_preferences->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 380, 430, "");
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 170, 380, 90, 30, _("Apply"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormPreferencesApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 380, 90, 30, _("Cancel"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormPreferencesCancelCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 70, 380, 90, 30, _("Ok"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormPreferencesOKCB, 0);
  fdui->tabfolder_prefs = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 0, 0, 380, 360, "");
    fl_set_object_boxtype(obj, FL_FLAT_BOX);
  fl_end_form();

  fdui->form_preferences->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

