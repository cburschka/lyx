/**
 * \file QPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
 
#include "ControlPrefs.h"
#include "QPrefsDialog.h"
#include "ui/QPrefAsciiModule.h"
#include "ui/QPrefDateModule.h"
#include "ui/QPrefKeyboardModule.h"
#include "ui/QPrefLatexModule.h"
#include "ui/QPrefScreenFontsModule.h"
#include "ui/QPrefColorsModule.h"
#include "ui/QPrefDisplayModule.h"
#include "ui/QPrefPathsModule.h"
#include "ui/QPrefSpellcheckerModule.h"
#include "ui/QPrefConvertersModule.h"
#include "ui/QPrefFileformatsModule.h"
#include "ui/QPrefLanguageModule.h"
#include "ui/QPrefPrinterModule.h"
#include "ui/QPrefUIModule.h"
#include "QPrefs.h"
#include "Qt2BC.h"
#include "lyxrc.h"

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
 
typedef Qt2CB<ControlPrefs, Qt2DB<QPrefsDialog> > base_class;


QPrefs::QPrefs()
	: base_class(_("LyX: Preferences"))
{
}


void QPrefs::build_dialog()
{
	dialog_.reset(new QPrefsDialog(this));

	bc().setOK(dialog_->savePB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().setRestore(dialog_->restorePB);
}


void QPrefs::apply()
{
	LyXRC & rc(controller().rc());

	// do something ... 

#if 0
	bool modifiedText = false;
	bool modifiedBackground = false;

	// Now do the same for the LyX LColors...
	for (vector<NamedColor>::const_iterator cit = lyxColorDB.begin();
	     cit != lyxColorDB.end(); ++cit) {
		LColor::color lc = lcolor.getFromGUIName(cit->getname());
		if (lc == LColor::inherit) continue;

		// Create a valid X11 name of the form "#rrggbb"
		string const hexname = X11hexname(cit->color());

		if (lcolor.getX11Name(lc) != hexname) {
			lyxerr[Debug::GUI]
				<< "FormPreferences::Colors::apply: "
				<< "resetting LColor " << lcolor.getGUIName(lc)
				<< " from \"" << lcolor.getX11Name(lc)
				<< "\" to \"" << hexname << "\"."
				<< endl;

			parent_.controller().setColor(lc, hexname);
		}
	}
}
 
	parent_.controller().setConverters(local_converters);

	parent_.controller().setFormats(local_formats);
 
	rc.date_insert_format =
		fl_get_input(dialog_->input_date_format);
 
	rc.popup_normal_font =
		fl_get_input(dialog_->input_popup_normal_font);
	rc.popup_bold_font = fl_get_input(dialog_->input_popup_bold_font);
	rc.popup_font_encoding =
		fl_get_input(dialog_->input_popup_font_encoding);
	rc.bind_file = fl_get_input(dialog_->input_bind_file);
	rc.ui_file = fl_get_input(dialog_->input_ui_file);
	rc.override_x_deadkeys =
		fl_get_button(dialog_->check_override_x_dead_keys);
}
	int const pos = combo_default_lang->get();
	rc.default_language = lang_[pos-1];

	int button = fl_get_button(dialog_->check_use_kbmap);
	string const name_1 = fl_get_input(dialog_->input_kbmap1);
	string const name_2 = fl_get_input(dialog_->input_kbmap2);
	if (button)
		button = !(name_1.empty() && name_2.empty());
	rc.use_kbmap = static_cast<bool>(button);

	if (rc.use_kbmap) {
		rc.primary_kbmap = name_1;
		rc.secondary_kbmap = name_2;
	}

	button = fl_get_button(dialog_->check_rtl_support);
	rc.rtl_support = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_mark_foreign);
	rc.mark_foreign_language = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_begin);
	rc.language_auto_begin = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_end);
	rc.language_auto_end = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_use_babel);
	rc.language_use_babel = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_global_options);
	rc.language_global_options = static_cast<bool>(button);

	rc.language_package = fl_get_input(dialog_->input_package);
	rc.language_command_begin = fl_get_input(dialog_->input_command_begin);
	rc.language_command_end = fl_get_input(dialog_->input_command_end);

	// Ensure that all is self-consistent.
	update(rc);

 
	rc.auto_region_delete =
		fl_get_button(dialog_->check_auto_region_delete);
	rc.cursor_follows_scrollbar =
		fl_get_button(dialog_->check_cursor_follows_scrollbar);
	rc.dialogs_iconify_with_main =
		fl_get_button(dialog_->check_dialogs_iconify_with_main);
	rc.preview = fl_get_button(dialog_->check_preview_latex);
	rc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_autosave));
	rc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_wm_jump));

	// See FIXME below
	// grfx::DisplayType old_value = rc.display_graphics;
	switch (fl_get_choice(dialog_->choice_display)) {
		case 4: rc.display_graphics = grfx::NoDisplay; break;
		case 3: rc.display_graphics = grfx::ColorDisplay; break;
		case 2: rc.display_graphics = grfx::GrayscaleDisplay; break;
		case 1: rc.display_graphics = grfx::MonochromeDisplay; break;
		default: rc.display_graphics = grfx::ColorDisplay; break;
	}

#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
	if (old_value != rc.display_graphics) {
		grfx::GCache & gc = grfx::GCache::get();
		gc.changeDisplay();
	}
#endif
 

 
	rc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_line_len));
	rc.fontenc = fl_get_input(dialog_->input_tex_encoding);

	int const choice =
		fl_get_choice(dialog_->choice_default_papersize) - 1;
	rc.default_papersize = static_cast<BufferParams::PAPER_SIZE>(choice);

	rc.ascii_roff_command = fl_get_input(dialog_->input_ascii_roff);
	rc.chktex_command = fl_get_input(dialog_->input_checktex);
	rc.view_dvi_paper_option = fl_get_input(dialog_->input_paperoption);
	rc.auto_reset_options = fl_get_button(dialog_->check_autoreset_classopt);


 
	rc.document_path = fl_get_input(dialog_->input_default_path);
	rc.template_path = fl_get_input(dialog_->input_template_path);

	int button = fl_get_button(dialog_->check_use_temp_dir);
	string str  = fl_get_input(dialog_->input_temp_dir);
	if (!button)
		str.erase();

	rc.use_tempdir = button;
	rc.tempdir_path = str;

	button = fl_get_button(dialog_->check_last_files);
	str = fl_get_input(dialog_->input_lastfiles);
	if (!button) str.erase();

	rc.check_lastfiles = button;
	rc.lastfiles = str;
	rc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_lastfiles));

	button = fl_get_button(dialog_->check_make_backups);
	str = fl_get_input(dialog_->input_backup_path);
	if (!button)
		str.erase();

	rc.make_backup = button;
	rc.backupdir_path = str;

	rc.lyxpipes = fl_get_input(dialog_->input_serverpipe);

	// update view
	update(rc);

 
	rc.print_adapt_output = fl_get_button(dialog_->check_adapt_output);
	rc.print_command = fl_get_input(dialog_->input_command);
	rc.print_pagerange_flag = fl_get_input(dialog_->input_page_range);
	rc.print_copies_flag = fl_get_input(dialog_->input_copies);
	rc.print_reverse_flag = fl_get_input(dialog_->input_reverse);
	rc.print_to_printer = fl_get_input(dialog_->input_to_printer);
	rc.print_file_extension =
		fl_get_input(dialog_->input_file_extension);
	rc.print_spool_command =
		fl_get_input(dialog_->input_spool_command);
	rc.print_paper_flag = fl_get_input(dialog_->input_paper_type);
	rc.print_evenpage_flag = fl_get_input(dialog_->input_even_pages);
	rc.print_oddpage_flag = fl_get_input(dialog_->input_odd_pages);
	rc.print_collcopies_flag = fl_get_input(dialog_->input_collated);
	rc.print_landscape_flag = fl_get_input(dialog_->input_landscape);
	rc.print_to_file = fl_get_input(dialog_->input_to_file);
	rc.print_extra_options =
		fl_get_input(dialog_->input_extra_options);
	rc.print_spool_printerprefix =
		fl_get_input(dialog_->input_spool_prefix);
	rc.print_paper_dimension_flag =
		fl_get_input(dialog_->input_paper_size);
	rc.printer = fl_get_input(dialog_->input_name);


 
	bool changed = false;

	string str = fl_get_input(dialog_->input_roman);
	if (rc.roman_font_name != str) {
		changed = true;
		rc.roman_font_name = str;
	}

	str = fl_get_input(dialog_->input_sans);
	if (rc.sans_font_name != str) {
		changed = true;
		rc.sans_font_name = str;
	}

	str = fl_get_input(dialog_->input_typewriter);
	if (rc.typewriter_font_name != str) {
		changed = true;
		rc.typewriter_font_name = str;
	}

	str = fl_get_input(dialog_->input_screen_encoding);
	if (rc.font_norm != str) {
		changed = true;
		rc.font_norm = str;
	}

	bool button = fl_get_button(dialog_->check_scalable);
	if (rc.use_scalable_fonts != button) {
		changed = true;
		rc.use_scalable_fonts = button;
	}

	unsigned int ivalue = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_zoom));
	if (rc.zoom != ivalue) {
		changed = true;
		rc.zoom = ivalue;
	}

	ivalue = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_dpi));
	if (rc.dpi != ivalue) {
		changed = true;
		rc.dpi = ivalue;
	}

	double dvalue = strToDbl(fl_get_input(dialog_->input_tiny));
	if (rc.font_sizes[LyXFont::SIZE_TINY] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_TINY] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_script));
	if (rc.font_sizes[LyXFont::SIZE_SCRIPT] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_SCRIPT] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_footnote));
	if (rc.font_sizes[LyXFont::SIZE_FOOTNOTE] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_FOOTNOTE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_small));
	if (rc.font_sizes[LyXFont::SIZE_SMALL] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_SMALL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_normal));
	if (rc.font_sizes[LyXFont::SIZE_NORMAL] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_NORMAL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_large));
	if (rc.font_sizes[LyXFont::SIZE_LARGE] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_LARGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_larger));
	if (rc.font_sizes[LyXFont::SIZE_LARGER] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_LARGER] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_largest));
	if (rc.font_sizes[LyXFont::SIZE_LARGEST] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_LARGEST] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_huge));
	if (rc.font_sizes[LyXFont::SIZE_HUGE] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_HUGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_huger));
	if (rc.font_sizes[LyXFont::SIZE_HUGER] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_HUGER] = dvalue;
	}

	if (changed) {
		// Now update the buffers
		// Can anything below here affect the redraw process?
		parent_.controller().updateScreenFonts();


 
	string choice = fl_get_choice_text(dialog_->choice_spell_command);
	choice = trim(choice);

	rc.isp_command = choice;

#if 0
	// If spell checker == "none", all other input set to off.
	if (fl_get_choice(dialog_->choice_spell_command) == 1) {
		rc.isp_use_alt_lang = false;
		rc.isp_alt_lang.erase();

		rc.isp_use_esc_chars = false;
		rc.isp_esc_chars.erase();

		rc.isp_use_pers_dict = false;
		rc.isp_pers_dict.erase();

		rc.isp_accept_compound = false;
		rc.isp_use_input_encoding = false;
	} else {
#else
		int button = fl_get_button(dialog_->check_alt_lang);
		choice = fl_get_input(dialog_->input_alt_lang);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		rc.isp_use_alt_lang = static_cast<bool>(button);
		rc.isp_alt_lang = choice;

		button = fl_get_button(dialog_->check_escape_chars);
		choice = fl_get_input(dialog_->input_escape_chars);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		rc.isp_use_esc_chars = static_cast<bool>(button);
		rc.isp_esc_chars = choice;

		button = fl_get_button(dialog_->check_personal_dict);
		choice = fl_get_input(dialog_->input_personal_dict);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		rc.isp_use_pers_dict = static_cast<bool>(button);
		rc.isp_pers_dict = choice;

		button = fl_get_button(dialog_->check_compound_words);
		rc.isp_accept_compound = static_cast<bool>(button);

		button = fl_get_button(dialog_->check_input_enc);
		rc.isp_use_input_encoding = static_cast<bool>(button);
#endif
#if 0
	}
#endif

	// Reset view
	update(rc);
#endif
 
}


void QPrefs::update_contents()
{
	LyXRC const & rc(controller().rc());

	QPrefLanguageModule * langmod(dialog_->languageModule);
 
	// FIXME: remove rtl_support bool
	langmod->rtlCB->setChecked(rc.rtl_support);
	langmod->markForeignCB->setChecked(rc.mark_foreign_language); 
	langmod->autoBeginCB->setChecked(rc.language_auto_begin); 
	langmod->autoEndCB->setChecked(rc.language_auto_end); 
	langmod->useBabelCB->setChecked(rc.language_use_babel);
	langmod->globalCB->setChecked(rc.language_global_options);
	langmod->languagePackageED->setText(rc.language_package.c_str());
	langmod->startCommandED->setText(rc.language_command_begin.c_str());
	langmod->endCommandED->setText(rc.language_command_end.c_str());


	QPrefUIModule * uimod(dialog_->uiModule); 

	uimod->uiFileED->setText(rc.bind_file.c_str());
	uimod->bindFileED->setText(rc.ui_file.c_str());
	uimod->cursorFollowsCB->setChecked(rc.cursor_follows_scrollbar); 
	uimod->wheelMouseSB->setValue(rc.wheel_jump);
	// convert to minutes
	int mins(rc.autosave / 60);
	if (rc.autosave && !mins)
		mins = 1;
	uimod->autoSaveSB->setValue(mins);
 
 
	QPrefKeyboardModule * keymod(dialog_->keyboardModule);

	// FIXME: can derive CB from the two EDs 
	keymod->keymapCB->setChecked(rc.use_kbmap);
	keymod->firstKeymapED->setText(rc.primary_kbmap.c_str());
	keymod->secondKeymapED->setText(rc.secondary_kbmap.c_str());
 

	QPrefAsciiModule * ascmod(dialog_->asciiModule);

	ascmod->asciiLinelengthSB->setValue(rc.ascii_linelen);
	ascmod->asciiRoffED->setText(rc.ascii_roff_command.c_str());


	QPrefDateModule * datemod(dialog_->dateModule);

	datemod->DateED->setText(rc.date_insert_format.c_str());


	QPrefLatexModule * latexmod(dialog_->latexModule);

	latexmod->latexEncodingED->setText(rc.fontenc.c_str());
	latexmod->latexChecktexED->setText(rc.chktex_command.c_str()); 
	latexmod->latexAutoresetCB->setChecked(rc.auto_reset_options);
	latexmod->latexDviPaperED->setText(rc.view_dvi_paper_option.c_str());

#if 0 
	local_converters = converters;
	local_converters.update(local_formats);
	UpdateBrowser();

	local_formats = formats;
	UpdateBrowser();

	int const pos = int(findPos(lang_, rc.default_language));
	combo_default_lang->select(pos + 1);

	fl_set_button(dialog_->check_preview_latex,
		      rc.preview);

	switch (rc.display_graphics) {
		case grfx::NoDisplay:		fl_set_choice(dialog_->choice_display, 4); break;
		case grfx::ColorDisplay:	fl_set_choice(dialog_->choice_display, 3); break;
		case grfx::GrayscaleDisplay:	fl_set_choice(dialog_->choice_display, 2); break;
		case grfx::MonochromeDisplay:	fl_set_choice(dialog_->choice_display, 1); break;
		default:			fl_set_choice(dialog_->choice_display, 3); break;
	}

	fl_set_choice(dialog_->choice_default_papersize,
		      rc.default_papersize + 1);

	fl_set_input(dialog_->input_default_path,
		     rc.document_path.c_str());
	fl_set_input(dialog_->input_template_path,
		     rc.template_path.c_str());

	string str;
	if (rc.make_backup)
		str = rc.backupdir_path;

	fl_set_button(dialog_->check_make_backups,
		      rc.make_backup);
	fl_set_input(dialog_->input_backup_path, str.c_str());

	str.erase();
	if (rc.use_tempdir)
		str = rc.tempdir_path;

	fl_set_button(dialog_->check_use_temp_dir,
		      rc.use_tempdir);
	fl_set_input(dialog_->input_temp_dir, str.c_str());

	str.erase();
	if (rc.check_lastfiles)
		str = rc.lastfiles;

	fl_set_button(dialog_->check_last_files,
		      rc.check_lastfiles);
	fl_set_input(dialog_->input_lastfiles, str.c_str());
	fl_set_counter_value(dialog_->counter_lastfiles,
			     rc.num_lastfiles);

	fl_set_input(dialog_->input_serverpipe, rc.lyxpipes.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);


	fl_set_button(dialog_->check_adapt_output,
		      rc.print_adapt_output);
	fl_set_input(dialog_->input_command,
		     rc.print_command.c_str());
	fl_set_input(dialog_->input_page_range,
		     rc.print_pagerange_flag.c_str());
	fl_set_input(dialog_->input_copies,
		     rc.print_copies_flag.c_str());
	fl_set_input(dialog_->input_reverse,
		     rc.print_reverse_flag.c_str());
	fl_set_input(dialog_->input_to_printer,
		     rc.print_to_printer.c_str());
	fl_set_input(dialog_->input_file_extension,
		     rc.print_file_extension.c_str());
	fl_set_input(dialog_->input_spool_command,
		     rc.print_spool_command.c_str());
	fl_set_input(dialog_->input_paper_type,
		     rc.print_paper_flag.c_str());
	fl_set_input(dialog_->input_even_pages,
		     rc.print_evenpage_flag.c_str());
	fl_set_input(dialog_->input_odd_pages,
		     rc.print_oddpage_flag.c_str());
	fl_set_input(dialog_->input_collated,
		     rc.print_collcopies_flag.c_str());
	fl_set_input(dialog_->input_landscape,
		     rc.print_landscape_flag.c_str());
	fl_set_input(dialog_->input_to_file,
		     rc.print_to_file.c_str());
	fl_set_input(dialog_->input_extra_options,
		     rc.print_extra_options.c_str());
	fl_set_input(dialog_->input_spool_prefix,
		     rc.print_spool_printerprefix.c_str());
	fl_set_input(dialog_->input_paper_size,
		     rc.print_paper_dimension_flag.c_str());
	fl_set_input(dialog_->input_name,
		     rc.printer.c_str());



	fl_set_input(dialog_->input_roman,
		     rc.roman_font_name.c_str());
	fl_set_input(dialog_->input_sans,
		     rc.sans_font_name.c_str());
	fl_set_input(dialog_->input_typewriter,
		     rc.typewriter_font_name.c_str());
	fl_set_counter_value(dialog_->counter_zoom, rc.zoom);
	fl_set_counter_value(dialog_->counter_dpi,  rc.dpi);
	fl_set_input(dialog_->input_tiny,
		     tostr(rc.font_sizes[LyXFont::SIZE_TINY]).c_str());
	fl_set_input(dialog_->input_script,
		     tostr(rc.font_sizes[LyXFont::SIZE_SCRIPT]).c_str());
	fl_set_input(dialog_->input_footnote,
		     tostr(rc.font_sizes[LyXFont::SIZE_FOOTNOTE]).c_str());
	fl_set_input(dialog_->input_small,
		     tostr(rc.font_sizes[LyXFont::SIZE_SMALL]).c_str());
	fl_set_input(dialog_->input_normal,
		     tostr(rc.font_sizes[LyXFont::SIZE_NORMAL]).c_str());
	fl_set_input(dialog_->input_large,
		     tostr(rc.font_sizes[LyXFont::SIZE_LARGE]).c_str());
	fl_set_input(dialog_->input_larger,
		     tostr(rc.font_sizes[LyXFont::SIZE_LARGER]).c_str());
	fl_set_input(dialog_->input_largest,
		     tostr(rc.font_sizes[LyXFont::SIZE_LARGEST]).c_str());
	fl_set_input(dialog_->input_huge,
		     tostr(rc.font_sizes[LyXFont::SIZE_HUGE]).c_str());
	fl_set_input(dialog_->input_huger,
		     tostr(rc.font_sizes[LyXFont::SIZE_HUGER]).c_str());

 
	int choice = 1;
#if 0
	if (rc.isp_command == "none")
		choice = 1;
	else if (rc.isp_command == "ispell")
		choice = 2;
	else if (rc.isp_command == "aspell")
		choice = 3;
#else
	if (rc.isp_command == "ispell")
		choice = 1;
	else if (rc.isp_command == "aspell")
		choice = 2;
#endif
	fl_set_choice(dialog_->choice_spell_command, choice);

	string str;
	if (rc.isp_use_alt_lang)
		str = rc.isp_alt_lang;

	fl_set_button(dialog_->check_alt_lang,
		      rc.isp_use_alt_lang);
	fl_set_input(dialog_->input_alt_lang, str.c_str());

	str.erase();
	if (rc.isp_use_esc_chars)
		str = rc.isp_esc_chars;

	fl_set_button(dialog_->check_escape_chars,
		      rc.isp_use_esc_chars);
	fl_set_input(dialog_->input_escape_chars, str.c_str());

	str.erase();
	if (rc.isp_use_pers_dict)
		str = rc.isp_pers_dict;

	fl_set_button(dialog_->check_personal_dict,
		      rc.isp_use_pers_dict);
	fl_set_input(dialog_->input_personal_dict, str.c_str());

	fl_set_button(dialog_->check_compound_words,
		      rc.isp_accept_compound);
	fl_set_button(dialog_->check_input_enc,
		      rc.isp_use_input_encoding);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
#endif 
}
