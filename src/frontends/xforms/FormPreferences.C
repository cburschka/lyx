/* FormPreferences.C
 * FormPreferences Interface Class Implementation
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include "FormPreferences.h"
#include "form_preferences.h"
#include "input_validators.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "debug.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif


FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, _("Preferences"), new PreferencesPolicy),
	  dialog_(0), outputs_tab_(0), look_n_feel_tab_(0), inputs_tab_(0),
	  lnf_general_(0), screen_fonts_(0), interface_(0),
	  printer_(0), paths_(0), outputs_general_(0), minw_(0), minh_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPreferences.connect(slot(this, &FormPreferences::show));
}


FormPreferences::~FormPreferences()
{
	delete dialog_;
	delete look_n_feel_tab_;
	delete inputs_tab_;
	delete outputs_tab_;
	delete lnf_general_;
	delete screen_fonts_;
	delete interface_;
	delete printer_;
	delete paths_;
	delete outputs_general_;
}


void FormPreferences::hide()
{
	// We need to hide the active tabfolder otherwise we get a
	// BadDrawable error from X windows and LyX crashes without saving.
	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabfolder_prefs);
	if (outer_form
	    && outer_form->visible) {
		fl_hide_form(outer_form);
	}
	FormBase::hide();
}


void FormPreferences::build()
{
	dialog_ = build_preferences();

	// manage the restore, save, apply and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.setUndoAll(dialog_->button_restore);
	bc_.refresh();

	// Workaround dumb xforms sizing bug
	minw_ = dialog_->form->w;
	minh_ = dialog_->form->h;

	// build the tab folders
	outputs_tab_ = build_outer_tab();
	look_n_feel_tab_ = build_outer_tab();
	inputs_tab_  = build_outer_tab();

	// build actual tabfolder contents
	// these will become nested tabfolders
	screen_fonts_ = build_screen_fonts();
	interface_ = build_interface();
	lnf_general_ = build_lnf_general();
	printer_ = build_printer();
	paths_ = build_paths();
	outputs_general_ = build_outputs_general();

	// setup the input returns
	// Lnf_General tab
	fl_set_counter_return(lnf_general_->counter_autosave, FL_RETURN_CHANGED);
	fl_set_counter_return(lnf_general_->counter_wm_jump, FL_RETURN_CHANGED);
	// Screen fonts
	fl_set_input_return(screen_fonts_->input_roman, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_sans, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_typewriter,
			    FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_screen_encoding,
			    FL_RETURN_CHANGED);
	fl_set_counter_return(screen_fonts_->counter_zoom, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_tiny, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_script, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_footnote, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_small, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_normal, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_large, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_larger, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_largest, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_huge, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_huger, FL_RETURN_CHANGED);
	// interface
	fl_set_input_return(interface_->input_popup_font,
			    FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_menu_font,
			    FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_popup_encoding,
			    FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_bind_file,
			    FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_ui_file,
			    FL_RETURN_CHANGED);
	// printer
	fl_set_input_return(printer_->input_command, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_page_range, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_copies, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_reverse, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_to_printer, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_file_extension, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_spool_command, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_paper_type, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_even_pages, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_odd_pages, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_collated, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_landscape, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_to_file, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_extra_options, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_spool_prefix, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_paper_size, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_name, FL_RETURN_CHANGED);
	// paths
	fl_set_input_return(paths_->input_default_path, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_template_path, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_temp_dir, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_backup_path, FL_RETURN_CHANGED);
	fl_set_counter_return(paths_->counter_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_serverpipe, FL_RETURN_CHANGED);
	// outputs general
	fl_set_counter_return(outputs_general_->counter_line_len,
			      FL_RETURN_CHANGED);

	// Now add them to the tabfolder
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Look and Feel"),
			   look_n_feel_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Inputs"),
			   inputs_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Outputs"),
			   outputs_tab_->form);

	// now build the nested tabfolders
	// Starting with look and feel
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Screen Fonts"),
			   screen_fonts_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Interface"),
			   interface_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("General"),
			   lnf_general_->form);

	// then build inputs
	// Paths should probably go in a few outer_tab called Files
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Paths"),
			   paths_->form);

	// then building outputs
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Printer"),
			   printer_->form);
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("General"),
			   outputs_general_->form);

	// deactivate the various browse buttons because they
	// currently aren't implemented
	fl_deactivate_object(interface_->button_bind_file_browse);
	fl_deactivate_object(interface_->button_ui_file_browse);
	fl_deactivate_object(paths_->button_document_browse);
	fl_deactivate_object(paths_->button_template_browse);
	fl_deactivate_object(paths_->button_temp_dir_browse);
	fl_deactivate_object(paths_->button_lastfiles_browse);
	fl_deactivate_object(paths_->button_backup_path_browse);
	fl_deactivate_object(paths_->button_serverpipe_browse);
	fl_set_object_lcol(interface_->button_bind_file_browse, FL_INACTIVE);
	fl_set_object_lcol(interface_->button_ui_file_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_document_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_template_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_temp_dir_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_lastfiles_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_backup_path_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_serverpipe_browse, FL_INACTIVE);
}


FL_FORM * FormPreferences::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormPreferences::connect()
{
	FormBaseBI::connect();
	fl_set_form_minsize(dialog_->form,
			    minw_,
			    minh_);
}


void FormPreferences::apply()
{
	// set the new lyxrc entries
	// many of these need to trigger other functions when the assignment
	// is made.  For example, screen zoom and font types.  These could be
	// handled either by signals/slots in lyxrc or just directly call the
	// associated functions here.
	// There are other problems with this scheme.  We really should check
	// what we copy to make sure that it really is necessary to do things
	// like update the screen fonts because that flushes the textcache
	// and other stuff which may cost us a lot on slower/high-load machines.
	
	// Look 'n Feel General tab
	lyxrc.show_banner = fl_get_button(lnf_general_->check_banner);
	lyxrc.auto_region_delete = fl_get_button(lnf_general_->
						 check_auto_region_delete);
	lyxrc.exit_confirmation = fl_get_button(lnf_general_->check_exit_confirm);
	lyxrc.display_shortcuts =
		fl_get_button(lnf_general_->check_display_shortcuts);
	lyxrc.new_ask_filename = fl_get_button(lnf_general_->check_ask_new_file);
	lyxrc.cursor_follows_scrollbar =
		fl_get_button(lnf_general_->check_cursor_follows_scrollbar);
	lyxrc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(lnf_general_->counter_autosave));
	lyxrc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(lnf_general_->counter_wm_jump));
	// Interface
	lyxrc.popup_font_name =
		fl_get_input(interface_->input_popup_font);
	lyxrc.menu_font_name = fl_get_input(interface_->input_menu_font);
	lyxrc.font_norm_menu =
		fl_get_input(interface_->input_popup_encoding);
	lyxrc.bind_file = fl_get_input(interface_->input_bind_file);
	lyxrc.ui_file = fl_get_input(interface_->input_ui_file);
	lyxrc.override_x_deadkeys =
		fl_get_button(interface_->check_override_x_dead_keys);
	// Screen fonts
	if (lyxrc.roman_font_name !=
	    fl_get_input(screen_fonts_->input_roman) ||
	    lyxrc.sans_font_name !=
	    fl_get_input(screen_fonts_->input_sans) ||
	    lyxrc.typewriter_font_name !=
	    fl_get_input(screen_fonts_->input_typewriter) ||
	    lyxrc.font_norm !=
	    fl_get_input(screen_fonts_->input_screen_encoding) ||
	    lyxrc.use_scalable_fonts !=
	    fl_get_button(screen_fonts_->check_scalable) ||
	    lyxrc.zoom != static_cast<unsigned int>
	    (fl_get_counter_value(screen_fonts_->counter_zoom)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_TINY] !=
	    strToDbl(fl_get_input(screen_fonts_->input_tiny)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] !=
	    strToDbl(fl_get_input(screen_fonts_->input_script)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] !=
	    strToDbl(fl_get_input(screen_fonts_->input_footnote)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_SMALL] !=
	    strToDbl(fl_get_input(screen_fonts_->input_small)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_NORMAL] !=
	    strToDbl(fl_get_input(screen_fonts_->input_normal)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_LARGE] !=
	    strToDbl(fl_get_input(screen_fonts_->input_large)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_LARGER] !=
	    strToDbl(fl_get_input(screen_fonts_->input_larger)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_LARGEST] !=
	    strToDbl(fl_get_input(screen_fonts_->input_largest)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_HUGE] !=
	    strToDbl(fl_get_input(screen_fonts_->input_huge)) ||
	    lyxrc.font_sizes[LyXFont::SIZE_HUGER] !=
	    strToDbl(fl_get_input(screen_fonts_->input_huger))) {
		// Something has changed so copy all of them and then force
		// an update of the screen fonts (ie. redraw every buffer)
		lyxrc.roman_font_name = fl_get_input(screen_fonts_->input_roman);
		lyxrc.sans_font_name = fl_get_input(screen_fonts_->input_sans);
		lyxrc.typewriter_font_name = fl_get_input(screen_fonts_->
							  input_typewriter);
		lyxrc.font_norm = fl_get_input(screen_fonts_->
					       input_screen_encoding);
		lyxrc.use_scalable_fonts =
			fl_get_button(screen_fonts_->check_scalable);
		lyxrc.zoom = static_cast<unsigned int>
			(fl_get_counter_value(screen_fonts_->counter_zoom));
		lyxrc.font_sizes[LyXFont::SIZE_TINY] = 
			strToDbl(fl_get_input(screen_fonts_->input_tiny));
		lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] =
			strToDbl(fl_get_input(screen_fonts_->input_script));
		lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] =
			strToDbl(fl_get_input(screen_fonts_->input_footnote));
		lyxrc.font_sizes[LyXFont::SIZE_SMALL] =
			strToDbl(fl_get_input(screen_fonts_->input_small));
		lyxrc.font_sizes[LyXFont::SIZE_NORMAL] =
			strToDbl(fl_get_input(screen_fonts_->input_normal));
		lyxrc.font_sizes[LyXFont::SIZE_LARGE] =
			strToDbl(fl_get_input(screen_fonts_->input_large));
		lyxrc.font_sizes[LyXFont::SIZE_LARGER] =
		strToDbl(fl_get_input(screen_fonts_->input_larger));
		lyxrc.font_sizes[LyXFont::SIZE_LARGEST] =
			strToDbl(fl_get_input(screen_fonts_->input_largest));
		lyxrc.font_sizes[LyXFont::SIZE_HUGE] =
			strToDbl(fl_get_input(screen_fonts_->input_huge));
		lyxrc.font_sizes[LyXFont::SIZE_HUGER] =
			strToDbl(fl_get_input(screen_fonts_->input_huger));
		// Now update the buffers
		// Can anything below here affect the redraw process?
		lv_->getLyXFunc()->Dispatch(LFUN_SCREEN_FONT_UPDATE);
	}
	// printer
	lyxrc.print_adapt_output = fl_get_button(printer_->check_adapt_output);
	lyxrc.print_command = fl_get_input(printer_->input_command);
	lyxrc.print_pagerange_flag = fl_get_input(printer_->input_page_range);
	lyxrc.print_copies_flag = fl_get_input(printer_->input_copies);
	lyxrc.print_reverse_flag = fl_get_input(printer_->input_reverse);
	lyxrc.print_to_printer = fl_get_input(printer_->input_to_printer);
	lyxrc.print_file_extension =
		fl_get_input(printer_->input_file_extension);
	lyxrc.print_spool_command =
		fl_get_input(printer_->input_spool_command);
	lyxrc.print_paper_flag = fl_get_input(printer_->input_paper_type);
	lyxrc.print_evenpage_flag = fl_get_input(printer_->input_even_pages);
	lyxrc.print_oddpage_flag = fl_get_input(printer_->input_odd_pages);
	lyxrc.print_collcopies_flag = fl_get_input(printer_->input_collated);
	lyxrc.print_landscape_flag = fl_get_input(printer_->input_landscape);
	lyxrc.print_to_file = fl_get_input(printer_->input_to_file);
	lyxrc.print_extra_options =
		fl_get_input(printer_->input_extra_options);
	lyxrc.print_spool_printerprefix =
		fl_get_input(printer_->input_spool_prefix);
	lyxrc.print_paper_dimension_flag =
		fl_get_input(printer_->input_paper_size);
	lyxrc.printer = fl_get_input(printer_->input_name);
	// paths
	lyxrc.document_path = fl_get_input(paths_->input_default_path);
	lyxrc.template_path = fl_get_input(paths_->input_template_path);
	lyxrc.tempdir_path = fl_get_input(paths_->input_temp_dir);
	lyxrc.lastfiles = fl_get_input(paths_->input_lastfiles);
	lyxrc.backupdir_path = fl_get_input(paths_->input_backup_path);
	lyxrc.use_tempdir = fl_get_button(paths_->check_use_temp_dir);
	lyxrc.check_lastfiles = fl_get_button(paths_->check_last_files);
	lyxrc.make_backup = fl_get_button(paths_->check_make_backups);
	lyxrc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(paths_->counter_lastfiles));
	lyxrc.lyxpipes = fl_get_input(paths_->input_serverpipe);
	// outputs general
	lyxrc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(outputs_general_->counter_line_len));
}


// we can safely ignore the parameter because we can always update
void FormPreferences::update(bool)
{
	if (dialog_) {
		// read lyxrc entries
		// Lnf_General tab
		fl_set_button(lnf_general_->check_banner,
			      lyxrc.show_banner);
		fl_set_button(lnf_general_->check_auto_region_delete,
			      lyxrc.auto_region_delete);
		fl_set_button(lnf_general_->check_exit_confirm,
			      lyxrc.exit_confirmation);
		fl_set_button(lnf_general_->check_display_shortcuts,
			      lyxrc.display_shortcuts);
		fl_set_button(lnf_general_->check_ask_new_file,
			      lyxrc.new_ask_filename);
		fl_set_button(lnf_general_->check_cursor_follows_scrollbar,
			      lyxrc.cursor_follows_scrollbar);
		fl_set_counter_value(lnf_general_->counter_autosave,
				     lyxrc.autosave);
		fl_set_counter_value(lnf_general_->counter_wm_jump,
				     lyxrc.wheel_jump);
		// Screen fonts
		fl_set_input(screen_fonts_->input_roman,
			     lyxrc.roman_font_name.c_str());
		fl_set_input(screen_fonts_->input_sans,
			     lyxrc.sans_font_name.c_str());
		fl_set_input(screen_fonts_->input_typewriter,
			     lyxrc.typewriter_font_name.c_str());
		fl_set_input(screen_fonts_->input_screen_encoding,
			     lyxrc.font_norm.c_str());
		fl_set_button(screen_fonts_->check_scalable,
			     lyxrc.use_scalable_fonts);
		fl_set_counter_value(screen_fonts_->counter_zoom,
			     lyxrc.zoom);
		fl_set_input(screen_fonts_->input_tiny,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_TINY]).c_str());
		fl_set_input(screen_fonts_->input_script,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_SCRIPT]).c_str());
		fl_set_input(screen_fonts_->input_footnote,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE]).c_str());
		fl_set_input(screen_fonts_->input_small,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_SMALL]).c_str());
		fl_set_input(screen_fonts_->input_normal,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_NORMAL]).c_str());
		fl_set_input(screen_fonts_->input_large,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGE]).c_str());
		fl_set_input(screen_fonts_->input_larger,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGER]).c_str());
		fl_set_input(screen_fonts_->input_largest,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGEST]).c_str());
		fl_set_input(screen_fonts_->input_huge,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_HUGE]).c_str());
		fl_set_input(screen_fonts_->input_huger,
			     tostr(lyxrc.font_sizes[LyXFont::SIZE_HUGER]).c_str());
		// interface
		fl_set_input(interface_->input_popup_font,
			     lyxrc.popup_font_name.c_str());
		fl_set_input(interface_->input_menu_font,
			     lyxrc.menu_font_name.c_str());
		fl_set_input(interface_->input_popup_encoding,
			     lyxrc.font_norm_menu.c_str());
		fl_set_input(interface_->input_bind_file,
			     lyxrc.bind_file.c_str());
		fl_set_input(interface_->input_ui_file,
			     lyxrc.ui_file.c_str());
		fl_set_button(interface_->check_override_x_dead_keys,
			      lyxrc.override_x_deadkeys);
		// printer
		fl_set_button(printer_->check_adapt_output,
			      lyxrc.print_adapt_output);
		fl_set_input(printer_->input_command,
			     lyxrc.print_command.c_str());
		fl_set_input(printer_->input_page_range,
			     lyxrc.print_pagerange_flag.c_str());
		fl_set_input(printer_->input_copies,
			     lyxrc.print_copies_flag.c_str());
		fl_set_input(printer_->input_reverse,
			      lyxrc.print_reverse_flag.c_str());
		fl_set_input(printer_->input_to_printer,
			     lyxrc.print_to_printer.c_str());
		fl_set_input(printer_->input_file_extension,
			     lyxrc.print_file_extension.c_str());
		fl_set_input(printer_->input_spool_command,
			     lyxrc.print_spool_command.c_str());
		fl_set_input(printer_->input_paper_type,
			     lyxrc.print_paper_flag.c_str());
		fl_set_input(printer_->input_even_pages,
			     lyxrc.print_evenpage_flag.c_str());
		fl_set_input(printer_->input_odd_pages,
			     lyxrc.print_oddpage_flag.c_str());
		fl_set_input(printer_->input_collated,
			     lyxrc.print_collcopies_flag.c_str());
		fl_set_input(printer_->input_landscape,
			     lyxrc.print_landscape_flag.c_str());
		fl_set_input(printer_->input_to_file,
			     lyxrc.print_to_file.c_str());
		fl_set_input(printer_->input_extra_options,
			     lyxrc.print_extra_options.c_str());
		fl_set_input(printer_->input_spool_prefix,
			     lyxrc.print_spool_printerprefix.c_str());
		fl_set_input(printer_->input_paper_size,
			     lyxrc.print_paper_dimension_flag.c_str());
		fl_set_input(printer_->input_name,
			     lyxrc.printer.c_str());
		// paths
		fl_set_input(paths_->input_default_path,
			     lyxrc.document_path.c_str());
		fl_set_input(paths_->input_template_path,
			     lyxrc.template_path.c_str());
		fl_set_input(paths_->input_temp_dir,
			     lyxrc.tempdir_path.c_str());
		fl_set_input(paths_->input_lastfiles,
			     lyxrc.lastfiles.c_str());
		fl_set_input(paths_->input_backup_path,
			     lyxrc.backupdir_path.c_str());
		fl_set_button(paths_->check_use_temp_dir,
			      lyxrc.use_tempdir);
		fl_set_button(paths_->check_last_files,
			      lyxrc.check_lastfiles);		
		fl_set_button(paths_->check_make_backups,
			      lyxrc.make_backup);
		fl_set_counter_value(paths_->counter_lastfiles,
				     lyxrc.num_lastfiles);
		fl_set_input(paths_->input_serverpipe, lyxrc.lyxpipes.c_str());
		// outputs general
		fl_set_counter_value(outputs_general_->counter_line_len,
				     lyxrc.ascii_linelen);

	}
}


bool FormPreferences::input(FL_OBJECT *, long)
{
	bool activate = true;
	//
	// whatever checks you need to ensure the user hasn't entered
	// some totally ridiculous value somewhere.  Change activate to suit.
	// comments before each test describe what is _valid_

	// input path -- dir should exist, be writable & absolute
	if (!AbsolutePath(fl_get_input(paths_->input_default_path))
	    || 1 != IsDirWriteable(fl_get_input(paths_->input_default_path))) {
		activate = false;
		lyxerr[Debug::GUI] << "Preferences: input path is wrong\n";
	}

	{
		// template_path should be a readable directory
		string temp(fl_get_input(paths_->input_template_path));
		FileInfo tp(temp);
		if (!AbsolutePath(temp)
		    || !tp.isDir()
		    || !tp.readable()) {
			activate = false;
			lyxerr[Debug::GUI] << "Preferences: template path is wrong\n";
		}
	}

	// tmpdir:  not used
	//          || writable directory
	if (fl_get_button(paths_->check_use_temp_dir)
	    && (1 != IsDirWriteable(fl_get_input(paths_->input_temp_dir))
		|| !AbsolutePath(fl_get_input(paths_->input_temp_dir)))) {
		activate = false;
		lyxerr[Debug::GUI] << "Preferences: tmpdir is wrong\n";
	}

	// backupdir: not used
	//            || empty
	//            || writable dir
	if (fl_get_button(paths_->check_make_backups)
	    && (!string(fl_get_input(paths_->input_backup_path)).empty()
		&& (1 != IsDirWriteable(fl_get_input(paths_->
						     input_backup_path))
		    || !AbsolutePath(fl_get_input(paths_->
						  input_backup_path))))) {
		activate = false;
		lyxerr[Debug::GUI] << "Preferences: backupdir is wrong\n";
	}

	// lastfiles: exists && writeable
	//            || non-existent && isn't a dir
#ifdef WITH_WARNINGS
#warning incorrectly allows files in /, other tests might also do this
#endif
	{
		string lastfiles(fl_get_input(paths_->input_lastfiles));
		FileInfo lf(lastfiles);
		if (!AbsolutePath(lastfiles)
		    || 1 != IsDirWriteable(OnlyPath(lastfiles))
		    || (lf.exist()
			&& (lf.isDir()
			    || !lf.writable()))) {
			activate = false;
			lyxerr[Debug::GUI] << "Preferences: lastfiles is wrong\n";
		}
	}

	// serverpipe:  empty
	//              || non-existent && isn't a dir
	//              || exists && writeable
	// remember we append .in and .out later
	if (!string(fl_get_input(paths_->input_serverpipe)).empty()) {
		string pipe(fl_get_input(paths_->input_serverpipe));
		FileInfo sp_in(pipe + ".in");
		FileInfo sp_out(pipe + ".out");
		if (!AbsolutePath(pipe)
		    || 1 != IsDirWriteable(OnlyPath(pipe))
		    || (sp_in.exist()
			&& (!sp_in.writable()
			    || sp_in.isDir()))
		    || (sp_out.exist()
			&& (!sp_out.writable()
			    || sp_out.isDir()))) {
			activate = false;
			lyxerr[Debug::GUI] << "Preferences: Serverpipe is wrong\n";
		}
	}

	//  fontsizes -- tiny < script < footnote etc.
	if (0.0 >= strToDbl(fl_get_input(screen_fonts_->input_tiny))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_script))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_footnote))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_small))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_normal))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_large))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_larger))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_largest))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_huge))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_huger))) {
		// make sure they all have positive entries
		// Also note that an empty entry is returned as 0.0 by strToDbl
		activate = false;
		lyxerr[Debug::GUI] << "Preferences: Sizes are wrong\n";
	} else if (strToDbl(fl_get_input(screen_fonts_->input_tiny)) >
		   strToDbl(fl_get_input(screen_fonts_->input_script)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_script)) >
		   strToDbl(fl_get_input(screen_fonts_->input_footnote)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_footnote)) >
		   strToDbl(fl_get_input(screen_fonts_->input_small)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_small)) >
		   strToDbl(fl_get_input(screen_fonts_->input_normal)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_normal)) >
		   strToDbl(fl_get_input(screen_fonts_->input_large)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_large)) >
		   strToDbl(fl_get_input(screen_fonts_->input_larger)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_larger)) >
		   strToDbl(fl_get_input(screen_fonts_->input_largest)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_largest)) >
		   strToDbl(fl_get_input(screen_fonts_->input_huge)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_huge)) >
		   strToDbl(fl_get_input(screen_fonts_->input_huger))) {
		activate = false;
		lyxerr[Debug::GUI] << "Preferences: Sizes are wrong\n";
	}

	return activate;
}


void FormPreferences::ok()
{
	FormBase::ok();
	lv_->getLyXFunc()->Dispatch(LFUN_SAVEPREFERENCES);
}


void FormPreferences::restore()
{
	update();
// if I add an error message line to the dialog it'll have to be controlled
// within input().  I don't need it yet so I'll leave it commented out.
//	bc_.valid(input(0));
}
