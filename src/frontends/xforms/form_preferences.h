/** Header file generated with fdesign on Fri Jul 14 09:35:06 2000.**/

#ifndef FD_form_bind_h_
#define FD_form_bind_h_

/** Callbacks, globals and object handlers **/






extern  "C" void C_FormPreferencesApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormPreferencesCancelCB(FL_OBJECT *, long);
extern  "C" void C_FormPreferencesOKCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_bind {

	FL_FORM *form_bind;
	FL_OBJECT *input_bind;
};
struct FD_form_misc {

	FL_FORM *form_misc;
	FL_OBJECT *check_banner;
	FL_OBJECT *check_auto_region_delete;
	FL_OBJECT *check_exit_confirm;
	FL_OBJECT *check_display_shortcuts;
	FL_OBJECT *counter_autosave;
	FL_OBJECT *counter_line_len;
};
struct FD_form_screen_fonts {

	FL_FORM *form_screen_fonts;
	FL_OBJECT *input_roman;
	FL_OBJECT *input_sans;
	FL_OBJECT *input_typewriter;
	FL_OBJECT *counter_zoom;
	FL_OBJECT *check_scalable;
	FL_OBJECT *input_encoding;
	FL_OBJECT *input_tiny;
	FL_OBJECT *input_script;
	FL_OBJECT *input_footnote;
	FL_OBJECT *input_small;
	FL_OBJECT *input_large;
	FL_OBJECT *input_larger;
	FL_OBJECT *input_largest;
	FL_OBJECT *input_normal;
	FL_OBJECT *input_huge;
	FL_OBJECT *input_huger;
};
struct FD_form_interface_fonts {

	FL_FORM *form_interface_fonts;
	FL_OBJECT *input_popup_font;
	FL_OBJECT *input_menu_font;
	FL_OBJECT *input_encoding;
};
struct FD_form_printer {

	FL_FORM *form_printer;
	FL_OBJECT *input_command;
	FL_OBJECT *input_page_range;
	FL_OBJECT *input_copies;
	FL_OBJECT *input_reverse;
	FL_OBJECT *input_to_printer;
	FL_OBJECT *input_file_extension;
	FL_OBJECT *input_spool_command;
	FL_OBJECT *input_paper_type;
	FL_OBJECT *input_even_pages;
	FL_OBJECT *input_odd_pages;
	FL_OBJECT *input_collated;
	FL_OBJECT *input_landscape;
	FL_OBJECT *input_to_file;
	FL_OBJECT *input_extra_options;
	FL_OBJECT *input_spool_prefix;
	FL_OBJECT *input_paper_size;
	FL_OBJECT *input_name;
	FL_OBJECT *check_adapt_output;
};
struct FD_form_paths {

	FL_FORM *form_paths;
	FL_OBJECT *input_default_path;
	FL_OBJECT *button_document_browse;
	FL_OBJECT *counter_lastfiles;
	FL_OBJECT *input_template_path;
	FL_OBJECT *button_template_browse;
	FL_OBJECT *check_last_files;
	FL_OBJECT *input_temp_dir;
	FL_OBJECT *button_temp_dir_browse;
	FL_OBJECT *input_lastfiles;
	FL_OBJECT *button_lastfiles_browse;
	FL_OBJECT *check_use_temp_dir;
	FL_OBJECT *check_make_backups;
	FL_OBJECT *input_backup_path;
	FL_OBJECT *button_backup_path_browse;
};
struct FD_form_preferences {

	FL_FORM *form_preferences;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *button_ok;
	FL_OBJECT *tabfolder_prefs;
};

#endif /* FD_form_bind_h_ */
