/* FormPreferences.C
 * FormPreferences Interface Class Implementation
 */

#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"
#include FORMS_H_LOCATION

#include "FormPreferences.h"
#include "xform_macros.h"
#include "input_validators.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "debug.h"


#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

C_RETURNCB(FormPreferences,  WMHideCB)
C_GENERICCB(FormPreferences, OKCB)
C_GENERICCB(FormPreferences, ApplyCB)
C_GENERICCB(FormPreferences, CancelCB)
C_GENERICCB(FormPreferences, InputCB)


FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: dialog_(0), bind_(0), misc_(0), screen_fonts_(0),
	  interface_fonts_(0), printer_(0), paths_(0),
	  lv_(lv), d_(d), u_(0), h_(0)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPreferences.connect(slot(this,&FormPreferences::show));
}


FormPreferences::~FormPreferences()
{
	free();
}


void FormPreferences::build()
{
	dialog_ = build_preferences();
	bind_ =	build_bind();
	screen_fonts_ = build_screen_fonts();
	interface_fonts_ = build_interface_fonts();
	misc_ = build_misc();
	printer_ = build_printer();
	paths_ = build_paths();

	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Bindings"),
			   bind_->form_bind);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Screen Fonts"),
			   screen_fonts_->form_screen_fonts);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Interface Fonts"),
			   interface_fonts_->form_interface_fonts);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Miscellaneous"),
			   misc_->form_misc);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Printer"),
			   printer_->form_printer);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Paths"),
			   paths_->form_paths);

	fl_set_form_atclose(dialog_->form_preferences,
			    C_FormPreferencesWMHideCB, 0);
}


void FormPreferences::show()
{
	if (!dialog_) {
		build();
	}
	update();  // make sure its up-to-date

	if (dialog_->form_preferences->visible) {
		fl_raise_form(dialog_->form_preferences);
	} else {
		fl_show_form(dialog_->form_preferences,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_FULLBORDER,
			     _("Preferences"));
	}
}


void FormPreferences::hide()
{
	if (dialog_
	    && dialog_->form_preferences
	    && dialog_->form_preferences->visible) {
		fl_hide_form(dialog_->form_preferences);
	}
}


void FormPreferences::apply()
{
	// set the new lyxrc entries
	// many of these need to trigger other functions when the assignment
	// is made.  For example, screen zoom and font types.  These could be
	// handled either by signals/slots in lyxrc or just directly call the
	// associated functions here.

	// Bind tab
	lyxrc.bind_file = fl_get_input(bind_->input_bind);
	// Misc tab
	lyxrc.show_banner = fl_get_button(misc_->check_banner);
	lyxrc.auto_region_delete = fl_get_button(misc_->
						 check_auto_region_delete);
	lyxrc.exit_confirmation = fl_get_button(misc_->check_exit_confirm);
	lyxrc.display_shortcuts = fl_get_button(misc_->check_display_shortcuts);
	lyxrc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(misc_->counter_autosave));
	lyxrc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(misc_->counter_line_len));
	// Screen fonts
	lyxrc.roman_font_name = fl_get_input(screen_fonts_->input_roman);
	lyxrc.sans_font_name = fl_get_input(screen_fonts_->input_sans);
	lyxrc.typewriter_font_name = fl_get_input(screen_fonts_->
						  input_typewriter);
	lyxrc.font_norm = fl_get_input(screen_fonts_->input_encoding);
	lyxrc.use_scalable_fonts = fl_get_button(screen_fonts_->check_scalable);
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
	// interface fonts
	lyxrc.popup_font_name =
		fl_get_input(interface_fonts_->input_popup_font);
	lyxrc.menu_font_name = fl_get_input(interface_fonts_->input_menu_font);
	lyxrc.font_norm_menu = fl_get_input(interface_fonts_->input_encoding);
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
}


void FormPreferences::update()
{
	if (dialog_) {
		// read lyxrc entries
		// Bind tab
		fl_set_input(bind_->input_bind, lyxrc.bind_file.c_str());
		// Misc tab
		fl_set_button(misc_->check_banner,
			      lyxrc.show_banner);
		fl_set_button(misc_->check_auto_region_delete,
			      lyxrc.auto_region_delete);
		fl_set_button(misc_->check_exit_confirm,
			      lyxrc.exit_confirmation);
		fl_set_button(misc_->check_display_shortcuts,
			      lyxrc.display_shortcuts);
		fl_set_counter_value(misc_->counter_autosave,
				     lyxrc.autosave);
		fl_set_counter_value(misc_->counter_line_len,
				     lyxrc.ascii_linelen);
		// Screen fonts
		fl_set_input(screen_fonts_->input_roman,
			     lyxrc.roman_font_name.c_str());
		fl_set_input(screen_fonts_->input_sans,
			     lyxrc.sans_font_name.c_str());
		fl_set_input(screen_fonts_->input_typewriter,
			     lyxrc.typewriter_font_name.c_str());
		fl_set_input(screen_fonts_->input_encoding,
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
		// interface fonts
		fl_set_input(interface_fonts_->input_popup_font,
			     lyxrc.popup_font_name.c_str());
		fl_set_input(interface_fonts_->input_menu_font,
			     lyxrc.menu_font_name.c_str());
		fl_set_input(interface_fonts_->input_encoding,
			     lyxrc.font_norm_menu.c_str());
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
	}
}


void FormPreferences::input()
{
	bool activate = true;
	//
	// whatever checks you need
	//
	if (activate) {
		fl_activate_object(dialog_->button_ok);
		fl_activate_object(dialog_->button_apply);
		fl_set_object_lcol(dialog_->button_ok, FL_BLACK);
		fl_set_object_lcol(dialog_->button_apply, FL_BLACK);
	} else {
		fl_deactivate_object(dialog_->button_ok);
		fl_deactivate_object(dialog_->button_apply);
		fl_set_object_lcol(dialog_->button_ok, FL_INACTIVE);
		fl_set_object_lcol(dialog_->button_apply, FL_INACTIVE);
	}
}


void FormPreferences::free()
{
	// we don't need to delete u and h here because
	// hide() does that after disconnecting.
	if (dialog_) {
		if (dialog_->form_preferences
		    && dialog_->form_preferences->visible) {
			hide();
		}
		fl_free_form(dialog_->form_preferences);
		delete dialog_;
		dialog_ = 0;
	}
}


int FormPreferences::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormPreferences * pre = static_cast<FormPreferences*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormPreferences::OKCB(FL_OBJECT * ob, long)
{
	FormPreferences * pre = static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();

	pre->lv_->getLyXFunc()->Dispatch(LFUN_SAVEPREFERENCES);
}


void FormPreferences::ApplyCB(FL_OBJECT * ob, long)
{
	FormPreferences * pre = static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->apply();
}


void FormPreferences::CancelCB(FL_OBJECT * ob, long)
{
	FormPreferences * pre = static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->hide();
}


void FormPreferences::InputCB(FL_OBJECT * ob, long)
{
	FormPreferences * pre = static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->input();
}



