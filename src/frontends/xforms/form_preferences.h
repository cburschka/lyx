// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
/** Header file generated with fdesign **/

#ifndef FD_form_preferences_h_
#define FD_form_preferences_h_

/** Callbacks, globals and object handlers **/
extern  "C" void C_FormBaseDeprecatedRestoreCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedOKCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedApplyCB(FL_OBJECT *, long);
extern  "C" void C_FormBaseDeprecatedCancelCB(FL_OBJECT *, long);


extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);

extern  "C" void C_FormBaseDeprecatedInputCB(FL_OBJECT *, long);


/**** Forms and Objects ****/
struct FD_form_preferences {
	~FD_form_preferences();

	FL_FORM *form;
	FL_OBJECT *tabfolder_prefs;
	FL_OBJECT *button_restore;
	FL_OBJECT *button_ok;
	FL_OBJECT *button_apply;
	FL_OBJECT *button_cancel;
	FL_OBJECT *text_warning;
};
struct FD_form_outer_tab {
	~FD_form_outer_tab();

	FL_FORM *form;
	FL_OBJECT *tabfolder_outer;
};
struct FD_form_screen_fonts {
	~FD_form_screen_fonts();

	FL_FORM *form;
	FL_OBJECT *input_roman;
	FL_OBJECT *input_sans;
	FL_OBJECT *input_typewriter;
	FL_OBJECT *input_screen_encoding;
	FL_OBJECT *check_scalable;
	FL_OBJECT *counter_zoom;
	FL_OBJECT *counter_dpi;
	FL_OBJECT *input_tiny;
	FL_OBJECT *input_script;
	FL_OBJECT *input_footnote;
	FL_OBJECT *input_small;
	FL_OBJECT *input_normal;
	FL_OBJECT *input_large;
	FL_OBJECT *input_larger;
	FL_OBJECT *input_largest;
	FL_OBJECT *input_huge;
	FL_OBJECT *input_huger;
};
struct FD_form_interface {
	~FD_form_interface();

	FL_FORM *form;
	FL_OBJECT *input_popup_font;
	FL_OBJECT *input_menu_font;
	FL_OBJECT *input_popup_encoding;
	FL_OBJECT *input_ui_file;
	FL_OBJECT *button_ui_file_browse;
	FL_OBJECT *input_bind_file;
	FL_OBJECT *button_bind_file_browse;
	FL_OBJECT *check_override_x_dead_keys;
};
struct FD_form_colors {
	~FD_form_colors();

	FL_FORM *form;
	FL_OBJECT *browser_lyx_objs;
	FL_OBJECT *dial_hue;
	FL_OBJECT *slider_saturation;
	FL_OBJECT *slider_value;
	FL_OBJECT *slider_red;
	FL_OBJECT *slider_green;
	FL_OBJECT *slider_blue;
	FL_OBJECT *button_colorspace;
	FL_OBJECT *text_color_values;
	FL_OBJECT *button_color;
	FL_OBJECT *button_modify;
};
struct FD_form_lnf_misc {
	~FD_form_lnf_misc();

	FL_FORM *form;
	FL_OBJECT *check_banner;
	FL_OBJECT *check_auto_region_delete;
	FL_OBJECT *check_exit_confirm;
	FL_OBJECT *check_display_shrtcuts;
	FL_OBJECT *check_ask_new_file;
	FL_OBJECT *check_cursor_follows_scrollbar;
	FL_OBJECT *check_dialogs_iconify_with_main;
	FL_OBJECT *counter_wm_jump;
	FL_OBJECT *counter_autosave;
	FL_OBJECT *radio_button_group_display;
	FL_OBJECT *radio_display_monochrome;
	FL_OBJECT *radio_display_grayscale;
	FL_OBJECT *radio_display_color;
	FL_OBJECT *radio_no_display;
};
struct FD_form_spelloptions {
	~FD_form_spelloptions();

	FL_FORM *form;
	FL_OBJECT *choice_spell_command;
	FL_OBJECT *check_alt_lang;
	FL_OBJECT *input_alt_lang;
	FL_OBJECT *check_escape_chars;
	FL_OBJECT *input_escape_chars;
	FL_OBJECT *check_personal_dict;
	FL_OBJECT *input_personal_dict;
	FL_OBJECT *button_personal_dict;
	FL_OBJECT *check_compound_words;
	FL_OBJECT *check_input_enc;
};
struct FD_form_language {
	~FD_form_language();

	FL_FORM *form;
	FL_OBJECT *input_package;
	FL_OBJECT *choice_default_lang;
	FL_OBJECT *check_use_kbmap;
	FL_OBJECT *input_kbmap1;
	FL_OBJECT *input_kbmap2;
	FL_OBJECT *button_kbmap1_browse;
	FL_OBJECT *button_kbmap2_browse;
	FL_OBJECT *check_rtl_support;
	FL_OBJECT *check_auto_begin;
	FL_OBJECT *check_use_babel;
	FL_OBJECT *check_mark_foreign;
	FL_OBJECT *check_auto_end;
	FL_OBJECT *check_global_options;
	FL_OBJECT *input_command_begin;
	FL_OBJECT *input_command_end;
};
struct FD_form_formats {
	~FD_form_formats();

	FL_FORM *form;
	FL_OBJECT *browser_all;
	FL_OBJECT *input_format;
	FL_OBJECT *input_gui_name;
	FL_OBJECT *input_shrtcut;
	FL_OBJECT *input_extension;
	FL_OBJECT *input_viewer;
	FL_OBJECT *button_add;
	FL_OBJECT *button_delete;
};
struct FD_form_converters {
	~FD_form_converters();

	FL_FORM *form;
	FL_OBJECT *browser_all;
	FL_OBJECT *choice_from;
	FL_OBJECT *choice_to;
	FL_OBJECT *input_converter;
	FL_OBJECT *input_flags;
	FL_OBJECT *button_add;
	FL_OBJECT *button_delete;
};
struct FD_form_paths {
	~FD_form_paths();

	FL_FORM *form;
	FL_OBJECT *input_default_path;
	FL_OBJECT *button_default_path_browse;
	FL_OBJECT *input_template_path;
	FL_OBJECT *button_template_path_browse;
	FL_OBJECT *check_use_temp_dir;
	FL_OBJECT *input_temp_dir;
	FL_OBJECT *button_temp_dir_browse;
	FL_OBJECT *check_last_files;
	FL_OBJECT *input_lastfiles;
	FL_OBJECT *button_lastfiles_browse;
	FL_OBJECT *counter_lastfiles;
	FL_OBJECT *check_make_backups;
	FL_OBJECT *input_backup_path;
	FL_OBJECT *button_backup_path_browse;
	FL_OBJECT *input_serverpipe;
	FL_OBJECT *button_serverpipe_browse;
};
struct FD_form_inputs_misc {
	~FD_form_inputs_misc();

	FL_FORM *form;
	FL_OBJECT *input_date_format;
};
struct FD_form_printer {
	~FD_form_printer();

	FL_FORM *form;
	FL_OBJECT *input_name;
	FL_OBJECT *check_adapt_output;
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
};
struct FD_form_outputs_misc {
	~FD_form_outputs_misc();

	FL_FORM *form;
	FL_OBJECT *counter_line_len;
	FL_OBJECT *input_tex_encoding;
	FL_OBJECT *choice_default_papersize;
	FL_OBJECT *input_ascii_roff;
	FL_OBJECT *input_checktex;
	FL_OBJECT *input_paperoption;
	FL_OBJECT *check_autoreset_classopt;
};

#endif /* FD_form_preferences_h_ */
