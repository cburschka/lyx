/* FormPreferences.C
 * FormPreferences Interface Class Implementation
 */

#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include <vector>

#include "FormPreferences.h"
#include "form_preferences.h"
#include "input_validators.h"
#include "LyXView.h"
#include "language.h"
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

using std::find;
using std::vector;

extern Languages languages;
static vector<string> languageNames;

FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, _("Preferences"), new PreferencesPolicy),
	  dialog_(0), outputs_tab_(0), look_n_feel_tab_(0), inputs_tab_(0),
	  usage_tab_(0), colours_(0), inputs_misc_(0), interface_(0),
	  language_(0), lnf_misc_(0), outputs_misc_(0), paths_(0), printer_(0),
	  screen_fonts_(0), spellchecker_(0)
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
	delete usage_tab_;
	delete colours_;
	delete inputs_misc_;
	delete interface_;
	delete language_;
	delete lnf_misc_;
	delete outputs_misc_;
	delete paths_;
	delete printer_;
	delete screen_fonts_;
	delete spellchecker_;
}


FL_FORM * FormPreferences::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
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
	minw_ = form()->w;
	minh_ = form()->h;

	// build the tab folders
	outputs_tab_ = build_outer_tab();
	look_n_feel_tab_ = build_outer_tab();
	inputs_tab_ = build_outer_tab();
	usage_tab_ = build_outer_tab();

	// build actual tabfolder contents
	// these will become nested tabfolders
	buildColours();
	buildInputsMisc();
	buildInterface();
	buildLanguage();
	buildLnFmisc();
	buildOutputsMisc();
	buildPaths();
	buildPrinter();
	buildScreenFonts();
	buildSpellchecker();

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
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Usage"),
			   usage_tab_->form);

	// now build the nested tabfolders
	// Starting with look and feel
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Screen Fonts"),
			   screen_fonts_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Interface"),
			   interface_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Colours"),
			   colours_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Misc"),
			   lnf_misc_->form);

	// then build inputs
	// Paths should probably go in a few outer_tab called Files
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Paths"),
			   paths_->form);
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Misc"),
			   inputs_misc_->form);

	// then building outputs
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Printer"),
			   printer_->form);
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Misc"),
			   outputs_misc_->form);

	// then building usage
	fl_addto_tabfolder(usage_tab_->tabfolder_outer,
			   _("Spell checker"),
			   spellchecker_->form);
	fl_addto_tabfolder(usage_tab_->tabfolder_outer,
			   _("Language"),
			   language_->form);
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

	applyColours();
	applyInputsMisc();
	applyInterface();
	applyLanguage();
	applyLnFmisc();
	applyOutputsMisc();
	applyPaths();
	applyPrinter();
	applyScreenFonts();
	applySpellChecker();
}


void FormPreferences::update()
{
	if (!dialog_) return;
    
	// read lyxrc entries
	updateColours();
	updateInputsMisc();
	updateInterface();
	updateLanguage();
	updateLnFmisc();
	updateOutputsMisc();
	updatePaths();
	updatePrinter();
	updateScreenFonts();
	updateSpellChecker();
}


bool FormPreferences::input(FL_OBJECT * ob, long data)
{
	bool activate = true;

	// Remove any existing messages
	fl_set_object_label(dialog_->text_warning, " ");

	// whatever checks you need to ensure the user hasn't entered
	// some totally ridiculous value somewhere.  Change activate to suit.
	// comments before each test describe what is _valid_

	State cb = static_cast<State>( data );
	switch( cb ) {
	case COLOURS:
		feedbackColours( ob );
		break;

	case INPUTSMISC:
		feedbackInputsMisc( ob );
		break;

	case INTERFACE:
		feedbackInterface( ob );
		break;

	case LANGUAGE:
		feedbackLanguage( ob );
		if( ! inputLanguage( ob ) )
			activate = false;
		break;

	case LOOKNFEELMISC:
		feedbackLnFmisc( ob );
		break;

	case OUTPUTSMISC:
		feedbackOutputsMisc( ob );
		break;

	case PATHS:
		feedbackPaths( ob );
		if( ! inputPaths( ob ) )
			activate = false;
		break;

	case PRINTER:
		feedbackPrinter( ob );
		break;

	case SCREENFONTS:
		feedbackScreenFonts( ob );
		if( ! inputScreenFonts() )
			activate = false;
		break;

	case SPELLCHECKER:
		feedbackSpellChecker( ob );
		if( ! inputSpellChecker( ob ) )
			activate = false;
		break;

	case TABS:
		break;
	}

	return activate;
}


void FormPreferences::applyColours()
{
}


void FormPreferences::buildColours()
{
	colours_ = build_colours();
}


void FormPreferences::feedbackColours( FL_OBJECT const * const )
{
}


void FormPreferences::updateColours()
{
}


void FormPreferences::applyInputsMisc()
{
	lyxrc.date_insert_format = fl_get_input(inputs_misc_->input_date_format);
}


void FormPreferences::buildInputsMisc()
{
	inputs_misc_ = build_inputs_misc();

	fl_set_input_return(inputs_misc_->input_date_format,
			    FL_RETURN_CHANGED);
}


void FormPreferences::feedbackInputsMisc( FL_OBJECT const * const ob )
{
	string str;

	if( ob == inputs_misc_->input_date_format ) {
		str = N_("This accepts the normal strftime formats; see man strftime for full details.\nE.g.\"%A, %e. %B %Y\".");
	}

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


void FormPreferences::updateInputsMisc()
{
	fl_set_input(inputs_misc_->input_date_format,
		     lyxrc.date_insert_format.c_str());
}


void FormPreferences::applyInterface()
{
	lyxrc.popup_font_name =
		fl_get_input(interface_->input_popup_font);
	lyxrc.menu_font_name = fl_get_input(interface_->input_menu_font);
	lyxrc.font_norm_menu =
		fl_get_input(interface_->input_popup_encoding);
	lyxrc.bind_file = fl_get_input(interface_->input_bind_file);
	lyxrc.ui_file = fl_get_input(interface_->input_ui_file);
	lyxrc.override_x_deadkeys =
		fl_get_button(interface_->check_override_x_dead_keys);
}


void FormPreferences::buildInterface()
{
	interface_ = build_interface();

	fl_set_input_return(interface_->input_popup_font, FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_menu_font, FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_popup_encoding, 
			    FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_bind_file, FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_ui_file, FL_RETURN_CHANGED);

	// deactivate the browse buttons because they aren't implemented
	fl_deactivate_object(interface_->button_bind_file_browse);
	fl_deactivate_object(interface_->button_ui_file_browse);
	fl_set_object_lcol(interface_->button_bind_file_browse, FL_INACTIVE);
	fl_set_object_lcol(interface_->button_ui_file_browse, FL_INACTIVE);
}


void FormPreferences::feedbackInterface( FL_OBJECT const * const ob )
{
	string str;

	if( ob == interface_->input_popup_font ) {
		str = N_("The font for popups.");
	} else if ( ob == interface_->input_menu_font ) {
		str = N_("The font for menus (and groups titles in popups).");
	} else if ( ob == interface_->input_popup_encoding ) {
		str = N_("The norm for the menu/popups fonts.");
	} else if ( ob == interface_->input_bind_file ) {
		str = N_("Keybindings file. Can either specify an absolute path,\nor LyX will look in its global and local bind/ directories.");
	} else if ( ob == interface_->input_ui_file ) {
		str = N_("The  UI (user interface) file. Can either specify an absolute path,\nor LyX will look in its global and local ui/ directories.");
	} else if ( ob == interface_->check_override_x_dead_keys ) {
		str = N_("Set to true for LyX to take over the handling of the dead keys\n(a.k.a accent keys) that may be defined for your keyboard.");
	}

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


void FormPreferences::updateInterface()
{
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
}


void FormPreferences::applyLanguage()
{
	vector<string>::size_type choice =
		fl_get_choice(language_->choice_default_lang) - 1;
	lyxrc.default_language = languageNames[choice];

	int button = fl_get_button(language_->check_use_kbmap);
	lyxrc.use_kbmap = static_cast<bool>(button);

	if( button ) {
	    choice = fl_get_choice(language_->choice_kbmap_1) - 1;
	    lyxrc.primary_kbmap = languageNames[choice];

	    choice = fl_get_choice(language_->choice_kbmap_2) - 1;
	    lyxrc.secondary_kbmap = languageNames[choice];
	}
	
	button = fl_get_button(language_->check_rtl_support);
	lyxrc.rtl_support = static_cast<bool>(button);

	button = fl_get_button(language_->check_auto_begin);
	lyxrc.language_auto_begin = static_cast<bool>(button);

	button = fl_get_button(language_->check_auto_end);
	lyxrc.language_auto_end = static_cast<bool>(button);

	button = fl_get_button(language_->check_mark_foreign);
	lyxrc.mark_foreign_language = static_cast<bool>(button);

	lyxrc.language_package = fl_get_input(language_->input_package);
	lyxrc.language_command_begin = fl_get_input(language_->input_command_begin);
	lyxrc.language_command_end = fl_get_input(language_->input_command_end);
}


void FormPreferences::buildLanguage()
{
	language_ = build_language();

	fl_set_input_return(language_->input_package, FL_RETURN_CHANGED);
	fl_set_input_return(language_->input_command_begin, FL_RETURN_CHANGED);
	fl_set_input_return(language_->input_command_end, FL_RETURN_CHANGED);

	// Only do this the first time buildLanguage() is called.
	if( languageNames.empty() ) {
		for( Languages::const_iterator lang = languages.begin();
		     lang != languages.end(); lang++ ) {
			languageNames.push_back( (*lang).second.lang() );
		}
	}

	string names = " " + languageNames.front();
	for( vector<string>::const_iterator lang = languageNames.begin();
	     lang != languageNames.end(); ++lang ) {
		names += " | " + (*lang);
	}
	names += " ";
	
	fl_addto_choice(language_->choice_default_lang, names.c_str());
	fl_addto_choice(language_->choice_kbmap_1, names.c_str());
	fl_addto_choice(language_->choice_kbmap_2, names.c_str());
}


void FormPreferences::feedbackLanguage( FL_OBJECT const * const ob )
{
	string str;

	if( ob == language_->choice_default_lang ) {
		str = N_("New documents will be assigned this language.");
	} else if( ob == language_->check_use_kbmap
		   || ob == language_->choice_kbmap_1
		   || ob == language_->choice_kbmap_2 ) {
		str = N_("Use this to set the correct mapping file for your keyboard.\nYou'll need this if you for instance want to type German documents\non an American keyboard.");
	} else if( ob == language_->check_rtl_support ) {
		str = N_("Use to enable support of right-to-left languages (e.g. Hebrew, Arabic).");
	} else if( ob == language_->check_auto_begin ) {
		str = N_("Use if a language switching command is needed at the beginning\nof the document.");
	} else if( ob == language_->check_auto_end ) {
		str = N_("Use if a language switching command is needed at the end\nof the document.");
	} else if( ob == language_->check_mark_foreign ) {
		str = N_("Use to control the highlighting of words with a language foreign to\nthat of the document.");
	} else if( ob == language_->input_package ) {
		str = N_("The latex command for loading the language package.\nE.g. \"\\usepackage{babel}\", \"\\usepackage{omega}\".");
	} else if( ob == language_->input_command_begin ) {
		str = N_("The latex command for changing from the language of the document\nto another language.\nE.g. \\selectlanguage{$$lang} where $$lang is substituted by the name\nof the second language.");
	} else if( ob == language_->input_command_end ) {
		str = N_("The latex command for changing back to the language of the document.");
	}

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


bool FormPreferences::inputLanguage( FL_OBJECT const * const ob )
{
	bool activate = true;
	
	if( !ob || ob == language_->check_use_kbmap ) {
		if( fl_get_button(language_->check_use_kbmap) ) {
			fl_activate_object(language_->choice_kbmap_1);
			fl_activate_object(language_->choice_kbmap_2);
			fl_set_object_lcol(language_->choice_kbmap_1,
					   FL_BLACK);
			fl_set_object_lcol(language_->choice_kbmap_2,
					   FL_BLACK);
		} else {
			fl_deactivate_object(language_->choice_kbmap_1);
			fl_deactivate_object(language_->choice_kbmap_2);
			fl_set_object_lcol(language_->choice_kbmap_1,
					   FL_INACTIVE);
			fl_set_object_lcol(language_->choice_kbmap_2,
					   FL_INACTIVE);
		}
	}

	return activate;
}


void FormPreferences::updateLanguage()
{
	fl_set_button(language_->check_use_kbmap,
		      lyxrc.use_kbmap);

	vector<string>::iterator it =
		find( languageNames.begin(), languageNames.end(),
		      lyxrc.default_language );
	int choice = 0;
	if( it != languageNames.end() )
		choice = static_cast<int>( it - languageNames.begin() );

	fl_set_choice(language_->choice_default_lang, choice+1);

	it = find( languageNames.begin(), languageNames.end(),
		   lyxrc.primary_kbmap );
	choice = 0;
	if( it != languageNames.end() )
		choice = static_cast<int>( it - languageNames.begin() );
	
	fl_set_choice(language_->choice_kbmap_1, choice+1);

	it = find( languageNames.begin(), languageNames.end(),
		   lyxrc.secondary_kbmap );
	choice = 0;
	if( it != languageNames.end() )
		choice = static_cast<int>( it - languageNames.begin() );

	fl_set_choice(language_->choice_kbmap_2, choice+1);
	
	fl_set_button(language_->check_rtl_support, lyxrc.rtl_support);
	fl_set_button(language_->check_auto_begin,  lyxrc.language_auto_begin);
	fl_set_button(language_->check_auto_end,    lyxrc.language_auto_end);
	fl_set_button(language_->check_mark_foreign,
		      lyxrc.mark_foreign_language);

	fl_set_input(language_->input_package,
		     lyxrc.language_package.c_str());
	fl_set_input(language_->input_command_begin,
		     lyxrc.language_command_begin.c_str());
	fl_set_input(language_->input_command_end,
		     lyxrc.language_command_end.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputLanguage( 0 );
}


void FormPreferences::applyLnFmisc()
{
	lyxrc.show_banner = fl_get_button(lnf_misc_->check_banner);
	lyxrc.auto_region_delete = fl_get_button(lnf_misc_->
						 check_auto_region_delete);
	lyxrc.exit_confirmation = fl_get_button(lnf_misc_->check_exit_confirm);
	lyxrc.display_shortcuts =
		fl_get_button(lnf_misc_->check_display_shortcuts);
	lyxrc.new_ask_filename = fl_get_button(lnf_misc_->check_ask_new_file);
	lyxrc.cursor_follows_scrollbar =
		fl_get_button(lnf_misc_->check_cursor_follows_scrollbar);
	lyxrc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(lnf_misc_->counter_autosave));
	lyxrc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(lnf_misc_->counter_wm_jump));
}


void FormPreferences::buildLnFmisc()
{
	lnf_misc_ = build_lnf_misc();

	fl_set_counter_return(lnf_misc_->counter_autosave,
			      FL_RETURN_CHANGED);
	fl_set_counter_return(lnf_misc_->counter_wm_jump,
			      FL_RETURN_CHANGED);
}


void FormPreferences::feedbackLnFmisc( FL_OBJECT const * const ob )
{
	string str;

	if( ob == lnf_misc_->check_banner ) {
		str = N_("Set to false if you don't want the startup banner.");
	} else if( ob == lnf_misc_->check_auto_region_delete ) {
		str = N_("Set to false if you don't want the current selection to be replaced\nautomatically by what you type.");
	} else if( ob == lnf_misc_->check_exit_confirm ) {
		str = N_("Sets whether LyX asks for a second confirmation to exit when you have\nchanged documents.\n(LyX will still ask to save changed documents.)");
	} else if( ob == lnf_misc_->check_display_shortcuts ) {
		str = N_("LyX continously displays names of last command executed,\nalong with a list of defined short-cuts for it in the minibuffer.\nSet to false if LyX seems slow.");
	} else if( ob == lnf_misc_->check_ask_new_file ) {
		str = N_("This sets the behaviour if you want to be asked for a filename when\ncreating a new document or wait until you save it and be asked then.");
	} else if( ob == lnf_misc_->check_cursor_follows_scrollbar ) {
		str = N_("LyX normally doesn't update the cursor position if you move the scrollbar.\nSet to true if you'd prefer to always have the cursor on screen.");
	} else if( ob == lnf_misc_->counter_autosave ) {
		str = N_("The time interval between auto-saves (in seconds).\n0 means no auto-save");
	} else if( ob == lnf_misc_->counter_wm_jump ) {
		str = N_("The wheel movement factor (for mice with wheels or five button mice)");
	} 
	
	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}

void FormPreferences::updateLnFmisc()
{
	fl_set_button(lnf_misc_->check_banner,
		      lyxrc.show_banner);
	fl_set_button(lnf_misc_->check_auto_region_delete,
		      lyxrc.auto_region_delete);
	fl_set_button(lnf_misc_->check_exit_confirm,
		      lyxrc.exit_confirmation);
	fl_set_button(lnf_misc_->check_display_shortcuts,
		      lyxrc.display_shortcuts);
	fl_set_button(lnf_misc_->check_ask_new_file,
		      lyxrc.new_ask_filename);
	fl_set_button(lnf_misc_->check_cursor_follows_scrollbar,
		      lyxrc.cursor_follows_scrollbar);
	fl_set_counter_value(lnf_misc_->counter_autosave,
			     lyxrc.autosave);
	fl_set_counter_value(lnf_misc_->counter_wm_jump,
			     lyxrc.wheel_jump);
}


void FormPreferences::applyOutputsMisc()
{
	lyxrc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(outputs_misc_->counter_line_len));
	lyxrc.fontenc = fl_get_input(outputs_misc_->input_tex_encoding);

	int choice =
		fl_get_choice(outputs_misc_->choice_default_papersize) - 1;
	lyxrc.default_papersize = static_cast<BufferParams::PAPER_SIZE>(choice);

	lyxrc.ascii_roff_command = fl_get_input(outputs_misc_->input_ascii_roff);
	lyxrc.chktex_command = fl_get_input(outputs_misc_->input_checktex);
}


void FormPreferences::buildOutputsMisc()
{
	outputs_misc_ = build_outputs_misc();

	fl_set_counter_return(outputs_misc_->counter_line_len,
			      FL_RETURN_CHANGED);
	fl_set_input_return(outputs_misc_->input_tex_encoding,
			    FL_RETURN_CHANGED);
	fl_set_input_return(outputs_misc_->input_ascii_roff,
			    FL_RETURN_CHANGED);
	fl_set_input_return(outputs_misc_->input_checktex,
			    FL_RETURN_CHANGED);
	fl_addto_choice(outputs_misc_->choice_default_papersize,
			_(" default | US letter | legal | executive | A3 | A4 | A5 | B5 "));
}


void FormPreferences::feedbackOutputsMisc(FL_OBJECT const * const ob )
{
	string str;

	if( ob == outputs_misc_->counter_line_len ) {
		str = N_("This is the maximum line length of an exported ASCII file\n(LaTeX, SGML or plain text).");
	} else if( ob == outputs_misc_->input_tex_encoding ) {
		str = N_("The font encoding used for the LaTeX2e fontenc package.\nT1 is highly recommended for non-English languages.");
	} else if( ob == outputs_misc_->input_ascii_roff ) {
		str = N_("Use to define an external program to render tables in the ASCII output.\nE.g. \"groff -t -Tlatin1 $$FName\"  where $$FName is the input file.\nIf \"none\" is specified, an internal routine is used.");
	} else if( ob == outputs_misc_->input_checktex ) {
		str = N_("Define how to run chktex.\nE.g. \"chktex -n11 -n1 -n3 -n6 -n9 -22 -n25 -n30 -n38\"\nRefer to the ChkTeX documentation.");
	} else if( ob == outputs_misc_->choice_default_papersize ) {
		str = N_("Specify the default paper size.");
	} 
		
	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


void FormPreferences::updateOutputsMisc()
{
	fl_set_counter_value(outputs_misc_->counter_line_len,
			     lyxrc.ascii_linelen);
	fl_set_input(outputs_misc_->input_tex_encoding,
		     lyxrc.fontenc.c_str());
	fl_set_choice(outputs_misc_->choice_default_papersize,
		      lyxrc.default_papersize+1);
	fl_set_input(outputs_misc_->input_ascii_roff,
		     lyxrc.ascii_roff_command.c_str());
	fl_set_input(outputs_misc_->input_checktex,
		     lyxrc.chktex_command.c_str());
}


void FormPreferences::applyPaths()
{
	lyxrc.document_path = fl_get_input(paths_->input_default_path);
	lyxrc.template_path = fl_get_input(paths_->input_template_path);

	int button = fl_get_button(paths_->check_use_temp_dir);
	string str  = fl_get_input(paths_->input_temp_dir);
	if( !button ) str.erase();

	lyxrc.use_tempdir = button;
	lyxrc.tempdir_path = str;

	button = fl_get_button(paths_->check_last_files);
	str = fl_get_input(paths_->input_lastfiles);
	if( !button ) str.erase();
	
	lyxrc.check_lastfiles = button;
	lyxrc.lastfiles = str;
	lyxrc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(paths_->counter_lastfiles));

	button = fl_get_button(paths_->check_make_backups);
	str = fl_get_input(paths_->input_backup_path);
	if( !button ) str.erase();

	lyxrc.make_backup = button;
	lyxrc.backupdir_path = str;

	lyxrc.lyxpipes = fl_get_input(paths_->input_serverpipe);

	// update view
	updatePaths();
}


void FormPreferences::buildPaths()
{
	paths_ = build_paths();

	fl_set_input_return(paths_->input_default_path, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_template_path, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_temp_dir, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_backup_path, FL_RETURN_CHANGED);
	fl_set_counter_return(paths_->counter_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_serverpipe, FL_RETURN_CHANGED);

	// deactivate the browse buttons because they aren't implemented
	fl_deactivate_object(paths_->button_document_browse);
	fl_deactivate_object(paths_->button_template_browse);
	fl_deactivate_object(paths_->button_temp_dir_browse);
	fl_deactivate_object(paths_->button_lastfiles_browse);
	fl_deactivate_object(paths_->button_backup_path_browse);
	fl_deactivate_object(paths_->button_serverpipe_browse);
	fl_set_object_lcol(paths_->button_document_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_template_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_temp_dir_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_lastfiles_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_backup_path_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_serverpipe_browse, FL_INACTIVE);
}


void FormPreferences::feedbackPaths( FL_OBJECT const * const ob )
{
	string str;
	if( ob == paths_->input_default_path ) {
		str = N_("The default path for your documents.");
	} else if ( ob == paths_->input_template_path ) {
		str = N_("The path that LyX will set when offering to choose a template.");
	} else if ( ob == paths_->check_use_temp_dir ) {
		str = N_("Specify to use a temporary directory to store temporary TeX output.\nThis directory is deleted when you quit LyX.");
	} else if ( ob == paths_->input_temp_dir ) {
		str = N_("The path that LyX will use to store temporary TeX output.");
	} else if ( ob == paths_->check_last_files ) {
		str = N_("Specify to check whether the lastfiles still exist.");
	} else if ( ob == paths_->input_lastfiles ) {
		str = N_("The file where the last-files information should be stored.");
	} else if ( ob == paths_->counter_lastfiles ) {
		str = N_("Maximal number of lastfiles. Up to 9 can appear in the file menu.");
	} else if ( ob == paths_->check_make_backups ) {
		str = N_("Set to false if you don't want LyX to create backup files.");
	} else if ( ob == paths_->input_backup_path ) {
		str = N_("The path for storing backup files. If it is an empty string,\nLyX will store the backup file in the same directory as the original file.");
	} else if ( ob == paths_->input_serverpipe ) {
		str = N_("This starts the lyxserver. The pipes get an additional extension\n\".in\" and \".out\". Only for advanced users.");
	}
	
	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


bool FormPreferences::inputPaths( FL_OBJECT const * const ob )
{
	bool activate = true;
	
	if( !ob || ob == paths_->check_use_temp_dir ) {
		if( fl_get_button(paths_->check_use_temp_dir) ) {
			fl_activate_object(paths_->input_temp_dir);
			fl_set_object_lcol(paths_->input_temp_dir,
					   FL_BLACK);
		} else {
			fl_deactivate_object(paths_->input_temp_dir);
			fl_set_object_lcol(paths_->input_temp_dir,
					   FL_INACTIVE);
		}
	}

	if( !ob || ob == paths_->check_last_files ) {
		if( fl_get_button(paths_->check_last_files) ) {
			fl_activate_object(paths_->input_lastfiles);
			fl_set_object_lcol(paths_->input_lastfiles,
					   FL_BLACK);
		} else {
			fl_deactivate_object(paths_->input_lastfiles);
			fl_set_object_lcol(paths_->input_lastfiles,
					   FL_INACTIVE);
		}
	}

	if( !ob || ob == paths_->check_make_backups ) {
		if( fl_get_button(paths_->check_make_backups) ) {
			fl_activate_object(paths_->input_backup_path);
			fl_set_object_lcol(paths_->input_backup_path,
					   FL_BLACK);
		} else {
			fl_deactivate_object(paths_->input_backup_path);
			fl_set_object_lcol(paths_->input_backup_path,
					   FL_INACTIVE);
		}
	}

	if( !ob || ob == paths_->input_default_path ) {
		string name = fl_get_input(paths_->input_default_path);
		if( !WriteableDir(name) )
			return false;
	}

	if( !ob || ob == paths_->input_template_path ) {
		string name = fl_get_input(paths_->input_template_path);
		if( !ReadableDir(name) )
		    return false;
	}

	if( !ob || ob == paths_->input_temp_dir ) {
		string name = fl_get_input(paths_->input_temp_dir);
		if( fl_get_button(paths_->check_make_backups)
		    && !name.empty()
		    && !WriteableDir(name) )
			return false;
	}

	if( !ob || ob == paths_->input_backup_path ) {
		string name = fl_get_input(paths_->input_backup_path);
		if( fl_get_button(paths_->check_make_backups)
		    && !name.empty()
		    && !WriteableDir(name) )
			return false;
	}

	if( !ob || ob == paths_->input_lastfiles ) {
		string name = fl_get_input(paths_->input_lastfiles);
		if( fl_get_button(paths_->check_last_files)
		    && !name.empty()
		    && !WriteableFile(name) )
			return false;
	}

	if( !ob || ob == paths_->input_serverpipe ) {
		string name = fl_get_input(paths_->input_serverpipe);
		if( !name.empty() ) {
			if( !WriteableFile(name, ".in") )
				return false;
			if( !WriteableFile(name, ".out") )
				return false;
		}
	}

	return activate;
}


void FormPreferences::updatePaths()
{
	fl_set_input(paths_->input_default_path,
		     lyxrc.document_path.c_str());
	fl_set_input(paths_->input_template_path,
		     lyxrc.template_path.c_str());

	string str = string();
	if( lyxrc.make_backup ) str = lyxrc.backupdir_path;

	fl_set_button(paths_->check_make_backups,
		      lyxrc.make_backup);
	fl_set_input(paths_->input_backup_path, str.c_str());

	str.erase();
	if( lyxrc.use_tempdir ) str = lyxrc.tempdir_path;

	fl_set_button(paths_->check_use_temp_dir,
		      lyxrc.use_tempdir);
	fl_set_input(paths_->input_temp_dir, str.c_str());

	str.erase();
	if( lyxrc.check_lastfiles ) str = lyxrc.lastfiles;

	fl_set_button(paths_->check_last_files,
		      lyxrc.check_lastfiles);		
	fl_set_input(paths_->input_lastfiles, str.c_str());
	fl_set_counter_value(paths_->counter_lastfiles,
			     lyxrc.num_lastfiles);

	fl_set_input(paths_->input_serverpipe, lyxrc.lyxpipes.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputPaths( 0 );
}


void FormPreferences::applyPrinter()
{
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
}


void FormPreferences::feedbackPrinter( FL_OBJECT const * const ob )
{
	string str;

	if( ob == printer_->input_command ) {
		str = N_("Your favorite print program, eg \"dvips\", \"dvilj4\"");
	} else if( ob == printer_->check_adapt_output ) {
		str = N_("Set to true for LyX to pass the name of the destination printer to your\nprint command.");
	} else if( ob == printer_->input_to_printer ) {
		str = N_("Option to pass to the print program to print on a specific printer.");
	} else if( ob == printer_->input_to_file ) {
		str = N_("Option to pass to the print program to print to a file.");
	} else if( ob == printer_->input_file_extension ) {
		str = N_("Extension of printer program output file. Usually .ps");
	} else if( ob == printer_->input_extra_options ) {
		str = N_("Extra options to pass to printing program after everything else,\nbut before the filename of the DVI file to be printed.");
	} else if( ob == printer_->input_spool_command ) {
		str = N_("When set, this printer option automatically prints to a file and then calls\na separate print spooling program on that file with the given name\nand arguments.");
	} else if( ob == printer_->input_spool_prefix ) {
		str = N_("If you specify a printer name in the print dialog, the following argument\nis prepended along with the printer name after the spool command.");
	} else if( ob == printer_->input_name ) {
		str = N_("The default printer to print on. If none is specified, LyX will\nuse the environment variable PRINTER.");
	} else if( ob == printer_->input_even_pages
		 || ob == printer_->input_odd_pages
		 || ob == printer_->input_page_range
		 || ob == printer_->input_reverse
		 || ob == printer_->input_landscape
		 || ob == printer_->input_copies
		 || ob == printer_->input_collated
		 || ob == printer_->input_paper_type
		 || ob == printer_->input_paper_size ) {
		str = N_("Look at the man page for your favorite print program to learn\nwhich options to use.");
	}
	
	
	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


void FormPreferences::buildPrinter()
{
	printer_ = build_printer();

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
}


void FormPreferences::updatePrinter()
{
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
}


void FormPreferences::applyScreenFonts()
{
	bool changed = false;

	string str = fl_get_input(screen_fonts_->input_roman);
	if( lyxrc.roman_font_name != str ) {
		changed = true;
		lyxrc.roman_font_name = str;
	}

	str = fl_get_input(screen_fonts_->input_sans);
	if( lyxrc.sans_font_name != str ) {
		changed = true;
		lyxrc.sans_font_name = str;
	}

	str = fl_get_input(screen_fonts_->input_typewriter);
	if( lyxrc.typewriter_font_name != str ) {
		changed = true;
		lyxrc.typewriter_font_name = str;
	}

	str = fl_get_input(screen_fonts_->input_screen_encoding);
	if( lyxrc.font_norm != str ) {
		changed = true;
		lyxrc.font_norm = str;
	}

	bool button = fl_get_button(screen_fonts_->check_scalable);
	if( lyxrc.use_scalable_fonts != button ) {
		changed = true;
		lyxrc.use_scalable_fonts = button;
	}

	int ivalue = fl_get_counter_value(screen_fonts_->counter_zoom);
	if( lyxrc.zoom != static_cast<unsigned int>( ivalue ) ) {
		changed = true;
		lyxrc.zoom = static_cast<unsigned int>( ivalue );
	}
	
	double dvalue = strToDbl(fl_get_input(screen_fonts_->input_tiny));
	if( lyxrc.font_sizes[LyXFont::SIZE_TINY] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_TINY] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_script));
	if( lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_footnote));
	if( lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_small));
	if( lyxrc.font_sizes[LyXFont::SIZE_SMALL] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_SMALL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_normal));
	if( lyxrc.font_sizes[LyXFont::SIZE_NORMAL] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_NORMAL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_large));
	if( lyxrc.font_sizes[LyXFont::SIZE_LARGE] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_larger));
	if( lyxrc.font_sizes[LyXFont::SIZE_LARGER] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGER] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_largest));
	if( lyxrc.font_sizes[LyXFont::SIZE_LARGEST] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGEST] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_huge));
	if( lyxrc.font_sizes[LyXFont::SIZE_HUGE] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_HUGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_huger));
	if( lyxrc.font_sizes[LyXFont::SIZE_HUGER] != dvalue ) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_HUGER] = dvalue;
	}

	if( changed ) {
		// Now update the buffers
		// Can anything below here affect the redraw process?
		lv_->getLyXFunc()->Dispatch(LFUN_SCREEN_FONT_UPDATE);
	}
}


void FormPreferences::buildScreenFonts()
{
	screen_fonts_ = build_screen_fonts();

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
}

	
void FormPreferences::feedbackScreenFonts(FL_OBJECT const * const ob )
{
	string str;

	if( ob == screen_fonts_->input_roman
	    || ob == screen_fonts_->input_sans
	    || ob == screen_fonts_->input_typewriter ) {
		str = N_("The screen fonts used to display the text while editing.");
	} else if( ob == screen_fonts_->check_scalable ) {
		str = N_("Allow the use of scalable screen fonts? If false, LyX will use the\nclosest existing size for a match. Use this if the scalable fonts\nlook bad and you have many fixed size fonts.");
	} else if( ob == screen_fonts_->input_screen_encoding ) {
		str = N_("The norm for the screen fonts.");
	} else if( ob == screen_fonts_->counter_zoom ) {
		str = N_("The zoom percentage for screen fonts.\nA setting of 100% will make the fonts roughly the same size as on paper.");
	} else if( ob == screen_fonts_->input_tiny
		   || ob == screen_fonts_->input_script
		   || ob == screen_fonts_->input_footnote
		   || ob == screen_fonts_->input_small
		   || ob == screen_fonts_->input_large
		   || ob == screen_fonts_->input_larger
		   || ob == screen_fonts_->input_larger
		   || ob == screen_fonts_->input_largest
		   || ob == screen_fonts_->input_normal
		   || ob == screen_fonts_->input_huge
		   || ob == screen_fonts_->input_huger ) {
		str = N_("The font sizes used for calculating the scaling of the screen fonts.");
	}
	
	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


bool FormPreferences::inputScreenFonts()
{
	bool activate = true;
	string str;

	// Make sure that all fonts all have positive entries
	// Also note that an empty entry is returned as 0.0 by strToDbl
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
		activate = false;
		str = N_("WARNING! Fonts must be positive!");

	// Fontsizes -- tiny < script < footnote etc.
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

		str = N_("WARNING! Fonts must be input in the order tiny > script>\nfootnote > small > normal > large > larger > largest > huge > huger.");
	}

	if( !activate ) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}
	
	return activate;
}


void FormPreferences::updateScreenFonts()
{
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
}


void FormPreferences::applySpellChecker()
{

	string choice = "none";
	switch(fl_get_choice(spellchecker_->choice_spell_command)) {
	case 1:
		choice = "none";
		break;
	case 2:
		choice = "ispell";
		break;
	case 3:
		choice = "aspell";
		break;
	default:
		break;
	}
	lyxrc.isp_command = choice;

	// If spell checker == "none", all other input set to off.
	if( fl_get_choice(spellchecker_->choice_spell_command) == 1 ) {
		lyxrc.isp_use_alt_lang = false;
		lyxrc.isp_alt_lang.erase();

		lyxrc.isp_use_esc_chars = false;
		lyxrc.isp_esc_chars.erase();

		lyxrc.isp_use_pers_dict = false;
		lyxrc.isp_pers_dict.erase();

		lyxrc.isp_accept_compound = false;
		lyxrc.isp_use_input_encoding = false;
	} else {
		int button = fl_get_button(spellchecker_->check_alt_lang);
		choice = fl_get_input(spellchecker_->input_alt_lang);
		if( button && choice.empty() ) button = 0;
		if( !button ) choice.erase();

		lyxrc.isp_use_alt_lang = static_cast<bool>(button);
		lyxrc.isp_alt_lang = choice;

		button = fl_get_button(spellchecker_->check_escape_chars);
		choice = fl_get_input(spellchecker_->input_escape_chars);
		if( button && choice.empty() ) button = 0;
		if( !button ) choice.erase();
	
		lyxrc.isp_use_esc_chars = static_cast<bool>(button);
		lyxrc.isp_esc_chars = choice;

		button = fl_get_button(spellchecker_->check_personal_dict);
		choice = fl_get_input(spellchecker_->input_personal_dict);
		if( button && choice.empty() ) button = 0;
		if( !button ) choice.erase();

		lyxrc.isp_use_pers_dict = static_cast<bool>(button);
		lyxrc.isp_pers_dict = choice;

		button = fl_get_button(spellchecker_->check_compound_words);
		lyxrc.isp_accept_compound = static_cast<bool>(button);

		button = fl_get_button(spellchecker_->check_input_enc);
		lyxrc.isp_use_input_encoding = static_cast<bool>(button);
	}

	// Reset view
	updateSpellChecker();
}


void FormPreferences::buildSpellchecker()
{
	spellchecker_ = build_spellchecker();

	fl_addto_choice(spellchecker_->choice_spell_command,
			_(" none | ispell | aspell "));
	fl_set_input_return(spellchecker_->input_alt_lang,
			    FL_RETURN_CHANGED);
	fl_set_input_return(spellchecker_->input_escape_chars,
			    FL_RETURN_CHANGED);
	fl_set_input_return(spellchecker_->input_personal_dict,
			    FL_RETURN_CHANGED);

	// deactivate the browse button because it isn't implemented
	fl_deactivate_object(spellchecker_->button_personal_dict);
	fl_set_object_lcol(spellchecker_->button_personal_dict,
			   FL_INACTIVE);
}


void FormPreferences::feedbackSpellChecker( FL_OBJECT const * const ob )
{
	string str;
	if( ob == spellchecker_->choice_spell_command ) {
		str = N_("What command runs the spell checker?");
	} else if( ob == spellchecker_->check_alt_lang ) {
		str = N_("Specify an alternate language.\nThe default is to use the language of the document.");
	} else if( ob == spellchecker_->check_escape_chars ) {
		str = N_("Specify additional chars that can be part of a word.");
	} else if( ob == spellchecker_->check_personal_dict ) {
		str = N_("Specify an alternate personal dictionary file.\nE.g. \".ispell_english\".");
	} else if( ob == spellchecker_->check_compound_words ) {
		str = N_("Consider run-together words, such as \"notthe\" for \"not the\",\nas legal words?");
	} else if( ob == spellchecker_->check_input_enc ) {
		str = N_("Specify whether to pass the -T input encoding option to ispell.\nEnable this if you can't spellcheck words with international letters\nin them.\nThis may not work with all dictionaries.");
	}

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


bool FormPreferences::inputSpellChecker( FL_OBJECT const * const ob )
{
	// Allow/dissallow input

	// If spell checker == "none", disable all input.
	if( !ob || ob == spellchecker_->choice_spell_command ) {
		if( fl_get_choice(spellchecker_->choice_spell_command) == 1 ) {
			fl_deactivate_object( spellchecker_->check_alt_lang );
			fl_deactivate_object( spellchecker_->input_alt_lang );
			fl_deactivate_object( spellchecker_->check_escape_chars );
			fl_deactivate_object( spellchecker_->input_escape_chars );
			fl_deactivate_object( spellchecker_->check_personal_dict );
			fl_deactivate_object( spellchecker_->input_personal_dict );
			fl_deactivate_object( spellchecker_->check_compound_words );
			fl_deactivate_object( spellchecker_->check_input_enc );
			return true;
		} else {
			fl_activate_object( spellchecker_->check_alt_lang );
			fl_activate_object( spellchecker_->check_escape_chars );
			fl_activate_object( spellchecker_->check_personal_dict );
			fl_activate_object( spellchecker_->check_compound_words );
			fl_activate_object( spellchecker_->check_input_enc );
		}
	}

	if( !ob || ob == spellchecker_->check_alt_lang ) {
		if( fl_get_button(spellchecker_->check_alt_lang) ) {
			fl_activate_object(spellchecker_->input_alt_lang);
			fl_set_object_lcol(spellchecker_->input_alt_lang,
					   FL_BLACK);
		} else {
			fl_deactivate_object(spellchecker_->input_alt_lang);
			fl_set_object_lcol(spellchecker_->input_alt_lang,
					   FL_INACTIVE);
		}
	}

	if( !ob || ob == spellchecker_->check_escape_chars ) {
		if( fl_get_button(spellchecker_->check_escape_chars) ) {
			fl_activate_object(spellchecker_->input_escape_chars);
			fl_set_object_lcol(spellchecker_->input_escape_chars,
					   FL_BLACK);
		} else {
			fl_deactivate_object(spellchecker_->input_escape_chars);
			fl_set_object_lcol(spellchecker_->input_escape_chars,
					   FL_INACTIVE);
		}
	}

	if( !ob || ob == spellchecker_->check_personal_dict ) {
		if( fl_get_button(spellchecker_->check_personal_dict) ) {
			fl_activate_object(spellchecker_->input_personal_dict);
			fl_set_object_lcol(spellchecker_->input_personal_dict,
					   FL_BLACK);
		} else {
			fl_deactivate_object(spellchecker_->input_personal_dict);
			fl_set_object_lcol(spellchecker_->input_personal_dict,
					   FL_INACTIVE);
		}
	}
	
	return true; // all input is valid!
}


void FormPreferences::updateSpellChecker()
{
	int choice = 1;
	if( lyxrc.isp_command == "none" )
		choice = 1;
	else if( lyxrc.isp_command == "ispell" )
		choice = 2;
	else if( lyxrc.isp_command == "aspell" )
		choice = 3;
	fl_set_choice(spellchecker_->choice_spell_command, choice);
	
	string str = string();
	if( lyxrc.isp_use_alt_lang ) str = lyxrc.isp_alt_lang;

	fl_set_button(spellchecker_->check_alt_lang,
		      lyxrc.isp_use_alt_lang);
	fl_set_input(spellchecker_->input_alt_lang, str.c_str());
	
	str.erase();
	if( lyxrc.isp_use_esc_chars ) str = lyxrc.isp_esc_chars;

	fl_set_button(spellchecker_->check_escape_chars,
		      lyxrc.isp_use_esc_chars);
	fl_set_input(spellchecker_->input_escape_chars, str.c_str());

	str.erase();
	if( lyxrc.isp_use_pers_dict ) str = lyxrc.isp_pers_dict;

	fl_set_button(spellchecker_->check_personal_dict,
		      lyxrc.isp_use_pers_dict);
	fl_set_input(spellchecker_->input_personal_dict, str.c_str());

	fl_set_button(spellchecker_->check_compound_words,
		      lyxrc.isp_accept_compound);
	fl_set_button(spellchecker_->check_input_enc,
		      lyxrc.isp_use_input_encoding);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputSpellChecker( 0 );
}


bool FormPreferences::WriteableDir( string const & name ) const
{
	bool success = true;
	string str;

	if( !AbsolutePath(name) ) {
		success = false;
		str = N_("WARNING! The absolute path is required.");
	}

	FileInfo tp(name);
	if( success && !tp.isDir() ) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}

	if( success && !tp.writable() ) {
		success = false;
		str = N_("WARNING! Cannot write to this directory.");
	}

	if( !success ) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}
	
	return success;
}


bool FormPreferences::ReadableDir( string const & name ) const
{
	bool success = true;
	string str;

	if( !AbsolutePath(name) ) {
		success = false;
		str = N_("WARNING! The absolute path is required.");
	}

	FileInfo tp(name);
	if( success && !tp.isDir() ) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}

	if( success && !tp.readable() ) {
		success = false;
		str = N_("WARNING! Cannot read this directory.");
	}

	if( !success ) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}

	return success;
}


bool FormPreferences::WriteableFile( string const & name, string const & suffix ) const
{
	// A writeable file is either:
	// * An existing file to which we have write access, or
	// * A file that doesn't yet exist but that would exist in a writeable
	//   directory.

	bool success = true;
	string str;

	if( name.empty() ) {
		success = false;
		str = N_("WARNING! No file input.");
	}

	string dir = OnlyPath(name);
	if( success && !AbsolutePath(dir) ) {
		success = false;
		str = N_("WARNING! The absolute path is required.");
	}

	FileInfo d;
	{
		FileInfo d1(dir);
		FileInfo d2(name);
		if( d2.isDir() )
			d = d2;
		else
			d = d1;
	}
	
	if( success && !d.isDir()) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}
	
	if( success && !d.writable() ) {
		success = false;
		str = N_("WARNING! Cannot write to this directory.");
	}

	FileInfo f(name+suffix);
	if( success && (dir == name || f.isDir()) ) {
		success = false;
		str = N_("WARNING! A file is required, not a directory.");
	}

	if( success && (f.exist() && !f.writable()) ) {
		success = false;
		str = N_("WARNING! Cannot write to this file.");
	}
	
	if( !success ) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}

	return success;
}
