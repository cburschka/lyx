/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxrc.h"
#endif

#include <fstream>
#include <iomanip>
#include <iostream>

#include "debug.h"

#include "lyxrc.h"
#include "kbmap.h"
#include "LyXAction.h"
#include "lyxserver.h"
#include "lyx_main.h"
#include "intl.h"
#include "tex-strings.h"
#include "support/path.h"
#include "support/filetools.h"
#include "lyxtext.h"
#include "converter.h"

using std::ostream;
using std::ofstream;
using std::cout;
using std::ios;
using std::endl;

// this is crappy... why are those colors command line arguments and
// not in lyxrc?? (Matthias) 
// Because nobody put them there. (Asger)

extern LyXAction lyxaction;
extern kb_keymap * toplevel_keymap;


static
keyword_item lyxrcTags[] = {
	{ "\\accept_compound", LyXRC::RC_ACCEPT_COMPOUND },
	{ "\\alternate_language", LyXRC::RC_ALT_LANG },
	{ "\\ascii_linelen", LyXRC::RC_ASCII_LINELEN },
	{ "\\ascii_roff_command", LyXRC::RC_ASCIIROFF_COMMAND },
	{ "\\auto_number", LyXRC::RC_AUTO_NUMBER },
	{ "\\auto_region_delete", LyXRC::RC_AUTOREGIONDELETE },
	{ "\\autosave", LyXRC::RC_AUTOSAVE },
	{ "\\backupdir_path", LyXRC::RC_BACKUPDIR_PATH },
	{ "\\bind", LyXRC::RC_BIND },
	{ "\\bind_file", LyXRC::RC_BINDFILE },
	{ "\\check_lastfiles", LyXRC::RC_CHECKLASTFILES },
	{ "\\chktex_command", LyXRC::RC_CHKTEX_COMMAND },
	{ "\\converter", LyXRC::RC_CONVERTER },
	{ "\\cursor_follows_scrollbar", LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR },
	{ "\\custom_export_command", LyXRC::RC_CUSTOM_EXPORT_COMMAND },
	{ "\\custom_export_format", LyXRC::RC_CUSTOM_EXPORT_FORMAT },
	{ "\\date_insert_format", LyXRC::RC_DATE_INSERT_FORMAT },
	{ "\\default_language", LyXRC::RC_DEFAULT_LANGUAGE },
	{ "\\default_papersize", LyXRC::RC_DEFAULT_PAPERSIZE },
	{ "\\display_shortcuts", LyXRC::RC_DISPLAY_SHORTCUTS },
	{ "\\document_path", LyXRC::RC_DOCUMENTPATH },
	{ "\\escape_chars", LyXRC::RC_ESC_CHARS },
	{ "\\exit_confirmation", LyXRC::RC_EXIT_CONFIRMATION },
	{ "\\fax_command", LyXRC::RC_FAX_COMMAND },
	{ "\\fax_program", LyXRC::RC_FAXPROGRAM },
	{ "\\font_encoding", LyXRC::RC_FONT_ENCODING },
	{ "\\format", LyXRC::RC_FORMAT },
	{ "\\input", LyXRC::RC_INPUT },
	{ "\\kbmap", LyXRC::RC_KBMAP },
	{ "\\kbmap_primary", LyXRC::RC_KBMAP_PRIMARY },
	{ "\\kbmap_secondary", LyXRC::RC_KBMAP_SECONDARY },
	{ "\\language_auto_begin", LyXRC::RC_LANGUAGE_AUTO_BEGIN },
	{ "\\language_auto_end", LyXRC::RC_LANGUAGE_AUTO_END },
	{ "\\language_command_begin", LyXRC::RC_LANGUAGE_COMMAND_BEGIN },
	{ "\\language_command_end", LyXRC::RC_LANGUAGE_COMMAND_END },
	{ "\\language_package", LyXRC::RC_LANGUAGE_PACKAGE },
	{ "\\lastfiles", LyXRC::RC_LASTFILES },
	{ "\\make_backup", LyXRC::RC_MAKE_BACKUP },
	{ "\\mark_foreign_language", LyXRC::RC_MARK_FOREIGN_LANGUAGE },
	{ "\\new_ask_filename", LyXRC::RC_NEW_ASK_FILENAME },
	{ "\\num_lastfiles", LyXRC::RC_NUMLASTFILES },
	{ "\\override_x_deadkeys", LyXRC::RC_OVERRIDE_X_DEADKEYS },
	{ "\\personal_dictionary", LyXRC::RC_PERS_DICT },
	{ "\\phone_book", LyXRC::RC_PHONEBOOK },
	{ "\\print_adapt_output", LyXRC::RC_PRINT_ADAPTOUTPUT },
	{ "\\print_collcopies_flag", LyXRC::RC_PRINTCOLLCOPIESFLAG },
	{ "\\print_command", LyXRC::RC_PRINT_COMMAND },
	{ "\\print_copies_flag", LyXRC::RC_PRINTCOPIESFLAG },
	{ "\\print_evenpage_flag", LyXRC::RC_PRINTEVENPAGEFLAG },
	{ "\\print_extra_options", LyXRC::RC_PRINTEXSTRAOPTIONS },
	{ "\\print_file_extension", LyXRC::RC_PRINTFILEEXTENSION },
	{ "\\print_landscape_flag", LyXRC::RC_PRINTLANDSCAPEFLAG },
	{ "\\print_oddpage_flag", LyXRC::RC_PRINTODDPAGEFLAG },
	{ "\\print_pagerange_flag", LyXRC::RC_PRINTPAGERANGEFLAG },
        { "\\print_paper_dimension_flag", LyXRC::RC_PRINTPAPERDIMENSIONFLAG },
        { "\\print_paper_flag", LyXRC::RC_PRINTPAPERFLAG },
	{ "\\print_reverse_flag", LyXRC::RC_PRINTREVERSEFLAG },
	{ "\\print_spool_command", LyXRC::RC_PRINTSPOOL_COMMAND },
	{ "\\print_spool_printerprefix", LyXRC::RC_PRINTSPOOL_PRINTERPREFIX },
	{ "\\print_to_file", LyXRC::RC_PRINTTOFILE },
	{ "\\print_to_printer", LyXRC::RC_PRINTTOPRINTER },
	{ "\\printer", LyXRC::RC_PRINTER },
	{ "\\ps_command", LyXRC::RC_PS_COMMAND },
	{ "\\rtl", LyXRC::RC_RTL_SUPPORT },
	{ "\\screen_dpi", LyXRC::RC_SCREEN_DPI },
	{ "\\screen_font_encoding", LyXRC::RC_SCREEN_FONT_ENCODING },
	{ "\\screen_font_encoding_menu", LyXRC::RC_SCREEN_FONT_ENCODING_MENU },
	{ "\\screen_font_menu", LyXRC::RC_SCREEN_FONT_MENU },
	{ "\\screen_font_popup", LyXRC::RC_SCREEN_FONT_POPUP },
	{ "\\screen_font_roman", LyXRC::RC_SCREEN_FONT_ROMAN },
	{ "\\screen_font_sans", LyXRC::RC_SCREEN_FONT_SANS },
	{ "\\screen_font_scalable", LyXRC::RC_SCREEN_FONT_SCALABLE },
	{ "\\screen_font_sizes", LyXRC::RC_SCREEN_FONT_SIZES },
	{ "\\screen_font_typewriter", LyXRC::RC_SCREEN_FONT_TYPEWRITER },
	{ "\\screen_zoom", LyXRC::RC_SCREEN_ZOOM },
	{ "\\serverpipe", LyXRC::RC_SERVERPIPE },
	{ "\\set_color", LyXRC::RC_SET_COLOR },
	{ "\\show_banner", LyXRC::RC_SHOW_BANNER },
	{ "\\spell_command", LyXRC::RC_SPELL_COMMAND },
	{ "\\tempdir_path", LyXRC::RC_TEMPDIRPATH },
	{ "\\template_path", LyXRC::RC_TEMPLATEPATH },
	{ "\\ui_file", LyXRC::RC_UIFILE },
	{ "\\use_alt_language", LyXRC::RC_USE_ALT_LANG },
	{ "\\use_escape_chars", LyXRC::RC_USE_ESC_CHARS },
	{ "\\use_input_encoding", LyXRC::RC_USE_INP_ENC },
	{ "\\use_personal_dictionary", LyXRC::RC_USE_PERS_DICT },
	{ "\\use_tempdir", LyXRC::RC_USETEMPDIR },
	{ "\\view_dvi_paper_option", LyXRC::RC_VIEWDVI_PAPEROPTION },
	{ "\\viewer" ,LyXRC::RC_VIEWER}, 
	{ "\\wheel_jump", LyXRC::RC_WHEEL_JUMP }
};

/* Let the range depend of the size of lyxrcTags.  Alejandro 240596 */
static const int lyxrcCount = sizeof(lyxrcTags) / sizeof(keyword_item);


LyXRC::LyXRC() 
{
	setDefaults();
}


void LyXRC::setDefaults() {
	ui_file = "default";
	// Get printer from the environment. If fail, use default "",
	// assuming that everything is set up correctly.
	printer = GetEnv("PRINTER");
	print_adapt_output = false;
	print_command = "dvips";
	print_evenpage_flag = "-B";
	print_oddpage_flag = "-A";
	print_pagerange_flag = "-pp";
	print_copies_flag = "-c";
	print_collcopies_flag = "-C";
	print_reverse_flag = "-r";
	print_landscape_flag = "-t landscape";
	print_to_printer = "-P";
	print_to_file = "-o ";
	print_file_extension = ".ps";
	print_paper_flag = "-t";
	print_paper_dimension_flag = "-T";
	document_path = GetEnvPath("HOME");
	tempdir_path = "/tmp";
	use_tempdir = true;
	ps_command = "gs";
	view_dvi_paper_option = "-paper";
	default_papersize = BufferParams::PAPER_USLETTER;
	custom_export_format = "ps";
	chktex_command = "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38";
	fontenc = "default";
	dpi = 75;
	// Because a screen typically is wider than a piece of paper:
	zoom = 150;
	wheel_jump = 100;
	// Default LaTeX font size:
	font_sizes[LyXFont::SIZE_TINY] = 5.0;
	font_sizes[LyXFont::SIZE_SCRIPT] = 7.0;
	font_sizes[LyXFont::SIZE_FOOTNOTE] = 8.0;
	font_sizes[LyXFont::SIZE_SMALL] = 9.0;
	font_sizes[LyXFont::SIZE_NORMAL] = 10.0;
	font_sizes[LyXFont::SIZE_LARGE] = 12.0;
	font_sizes[LyXFont::SIZE_LARGER] = 14.4;
	font_sizes[LyXFont::SIZE_LARGEST] = 17.26;
	font_sizes[LyXFont::SIZE_HUGE] = 20.74;
	font_sizes[LyXFont::SIZE_HUGER] = 24.88;
	use_scalable_fonts = true;
	roman_font_name = "-*-times";
	sans_font_name = "-*-helvetica";
	typewriter_font_name = "-*-courier";
	menu_font_name = "-*-helvetica-bold-r";
	popup_font_name = "-*-helvetica-medium-r";
	font_norm = "iso8859-1";
	font_norm_type = OTHER_ENCODING;
	font_norm_menu = "";
	override_x_deadkeys = true;
	autosave = 300;
	auto_region_delete = true;
	ascii_linelen = 65;
	num_lastfiles = 4;
	check_lastfiles = true;
	make_backup = true;
	backupdir_path = "";
	exit_confirmation = true;
	display_shortcuts = true;
	// Spellchecker settings:
	isp_command = "ispell";
	isp_accept_compound = false;
	isp_use_input_encoding = false;
	isp_use_alt_lang = false;
	isp_use_pers_dict = false;
	isp_use_esc_chars = false;
	use_kbmap = false;
	hasBindFile = false;
	rtl_support = false;
	auto_number = true;
	mark_foreign_language = true;
	language_package = "\\usepackage{babel}";
	language_auto_begin = true;
	language_auto_end = true;
	language_command_begin = "\\selectlanguage{$$lang}";
	language_command_end = "\\selectlanguage{$$lang}";
	default_language = "english";
	//
	new_ask_filename = false;

	//
	date_insert_format = "%A, %e %B %Y";
	show_banner = true;
	cursor_follows_scrollbar = false;

	/// These variables are not stored on disk (perhaps they
	// should be moved from the LyXRC class).
	use_gui = true;
	pdf_mode = false;
}


int LyXRC::ReadBindFile(string const & name)
{
	hasBindFile = true;
	string tmp = i18nLibFileSearch("bind", name, "bind");
	lyxerr[Debug::LYXRC] << "Reading bindfile:" << tmp << endl;
	int result = read(tmp);
	if (result) {
		lyxerr << "Error reading bind file: " << tmp << endl;
	}
	return result;
}


int LyXRC::read(string const & filename)
{
	// Default bindfile.
	string bindFile = "cua";
	
	LyXLex lexrc(lyxrcTags, lyxrcCount);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);
	
	lexrc.setFile(filename);
	if (!lexrc.IsOK()) return -2;
	
	lyxerr[Debug::INIT] << "Reading '" << filename << "'..." << endl;
	
	while (lexrc.IsOK()) {
		// By using two switches we take advantage of the compiler
		// telling us if we have missed a LyXRCTags element in
		// the second switch.
		// Note that this also shows a problem with LyXLex since it
		// helps us avoid taking advantage of the strictness of the
		// compiler.

		int le = lexrc.lex();
		switch(le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			continue; 
		case LyXLex::LEX_FEOF:
			continue; 
		default: break;
		}
		switch (static_cast<LyXRCTags>(le)) {
		case RC_INPUT: // Include file
		        if (lexrc.next()) {
				string tmp = LibFileSearch(string(),
							   lexrc.GetString()); 
				if (read(tmp)) {
					lexrc.printError("Error reading "
							 "included file: "+tmp);
				}
			}
			break;
		case RC_BINDFILE:                     // RVDK_PATCH_5
			if (lexrc.next()) {
				string tmp(lexrc.GetString());
				if (bind_file.empty()) {
					// we only need the name of the first
					// bind file since that (usually)
					// includes several others.
					bind_file = tmp;
				}
				ReadBindFile(tmp);
			}
			break;
			
		case RC_UIFILE: 
			if (lexrc.next()) {
				ui_file = lexrc.GetString();
			}
			break;
			
		case RC_EXIT_CONFIRMATION:
			if (lexrc.next())
				exit_confirmation = lexrc.GetBool();
			break;
			
		case RC_DISPLAY_SHORTCUTS:
			if (lexrc.next())
				display_shortcuts = lexrc.GetBool();
			break;
			
		case RC_KBMAP:
			if (lexrc.next())
				use_kbmap = lexrc.GetBool();
			break;
			
		case RC_KBMAP_PRIMARY:
			if (lexrc.next()) {
				string kmap(lexrc.GetString());
				if (kmap.empty()) {
					// nothing
				} else if (!LibFileSearch("kbd", kmap, 
							  "kmap").empty()) 
					primary_kbmap = kmap;
				else 
					lexrc.printError("LyX: Keymap `$$Token' not found");
			}
			break;
			
		case RC_KBMAP_SECONDARY:
			if (lexrc.next()) {
				string kmap(lexrc.GetString());
				if (kmap.empty()) {
					// nothing
				} else if (!LibFileSearch("kbd", kmap, 
							  "kmap").empty()) 
					secondary_kbmap = kmap;
				else 
					lexrc.printError("LyX: Keymap `$$Token' not found");
			}
			break;
			
		case RC_FONT_ENCODING:
			if (lexrc.next())
				fontenc = lexrc.GetString();
			break;
			
		case RC_PRINTER:
			if (lexrc.next())
				printer = lexrc.GetString();
			break;
			
		case RC_PRINT_COMMAND:
			if (lexrc.next())
				print_command = lexrc.GetString();
			break;
			
		case RC_PRINTEVENPAGEFLAG:
			if (lexrc.next())
				print_evenpage_flag = lexrc.GetString();
			break;
			
		case RC_PRINTODDPAGEFLAG:
			if (lexrc.next())
				print_oddpage_flag = lexrc.GetString();
			break;
			
		case RC_PRINTPAGERANGEFLAG:
			if (lexrc.next())
				print_pagerange_flag = lexrc.GetString();
			break;
			
		case RC_PRINTCOPIESFLAG:
			if (lexrc.next())
				print_copies_flag = lexrc.GetString();
			break;
			
		case RC_PRINTCOLLCOPIESFLAG:
			if (lexrc.next())
				print_collcopies_flag = lexrc.GetString();
			break;
			
		case RC_PRINTREVERSEFLAG:
			if (lexrc.next())
				print_reverse_flag = lexrc.GetString();
			break;
			
		case RC_PRINTLANDSCAPEFLAG:
			if (lexrc.next())
				print_landscape_flag = lexrc.GetString();
			break;
			
		case RC_PRINTTOPRINTER:
			if (lexrc.next())
				print_to_printer = lexrc.GetString();
			break;
			
		case RC_PRINT_ADAPTOUTPUT:
			if (lexrc.next())
				print_adapt_output = lexrc.GetBool();
			break;
			
		case RC_PRINTTOFILE:
			if (lexrc.next())
				print_to_file = lexrc.GetString();
			break;
			
		case RC_PRINTFILEEXTENSION:
			if (lexrc.next())
				print_file_extension = lexrc.GetString();
			break;
			
		case RC_PRINTEXSTRAOPTIONS:
			if (lexrc.next())
				print_extra_options = lexrc.GetString();
			break;
			
		case RC_PRINTSPOOL_COMMAND:
			if (lexrc.next())
				print_spool_command = lexrc.GetString();
			break;
			
		case RC_PRINTSPOOL_PRINTERPREFIX:
			if (lexrc.next())
				print_spool_printerprefix = lexrc.GetString();
			break;
			
                case RC_PRINTPAPERDIMENSIONFLAG:
			if (lexrc.next())
				print_paper_dimension_flag = lexrc.GetString();
			break;
			
                case RC_PRINTPAPERFLAG:
			if (lexrc.next())
				print_paper_flag = lexrc.GetString();
			break;
			
		case RC_CUSTOM_EXPORT_COMMAND:
			if (lexrc.next())
				custom_export_command = lexrc.GetString();
			break;
			
		case RC_CUSTOM_EXPORT_FORMAT:
			if (lexrc.next())
				custom_export_format = lexrc.GetString();
			break;

		case RC_DEFAULT_PAPERSIZE:
                        if (lexrc.next()) {
			        string size = lowercase(lexrc.GetString());
				if (size == "usletter")
				        default_papersize =
						BufferParams::PAPER_USLETTER;
				else if (size == "legal")
				        default_papersize =
						BufferParams::PAPER_LEGALPAPER;
				else if (size == "executive")
				        default_papersize =
						BufferParams::PAPER_EXECUTIVEPAPER;
				else if (size == "a3")
				        default_papersize =
						BufferParams::PAPER_A3PAPER;
				else if (size == "a4")
				        default_papersize =
						BufferParams::PAPER_A4PAPER;
				else if (size == "a5")
				        default_papersize =
						BufferParams::PAPER_A5PAPER;
				else if (size == "b5")
				        default_papersize =
						BufferParams::PAPER_B5PAPER;
			}
			break;

		case RC_VIEWDVI_PAPEROPTION:
			if (lexrc.next())
				view_dvi_paper_option = lexrc.GetString();
			else 
				view_dvi_paper_option = "";
			break;

		case RC_PS_COMMAND:
			if (lexrc.next())
				ps_command = lexrc.GetString();
			break;
			
		case RC_CHKTEX_COMMAND:
			if (lexrc.next())
				chktex_command = lexrc.GetString();
			break;
			
		case RC_SCREEN_DPI:
			if (lexrc.next())
				dpi = lexrc.GetInteger();
			break;
			
		case RC_SCREEN_ZOOM:
			if (lexrc.next())
				zoom = lexrc.GetInteger();
			break;

		case RC_WHEEL_JUMP:
			if (lexrc.next())
				wheel_jump = lexrc.GetInteger();
			break;
			
		case RC_SCREEN_FONT_SIZES:
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_TINY] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_SCRIPT] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_FOOTNOTE] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_SMALL] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_NORMAL] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_LARGE] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_LARGER] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_LARGEST] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_HUGE] =
					lexrc.GetFloat();
			if (lexrc.next())
				font_sizes[LyXFont::SIZE_HUGER] =
					lexrc.GetFloat();
			break;
			
                case RC_SCREEN_FONT_SCALABLE:
                        if (lexrc.next())
                                use_scalable_fonts = lexrc.GetBool();
			break;
			
		case RC_AUTOSAVE:
			if (lexrc.next())
				autosave = lexrc.GetInteger();
			break;
			
		case RC_DOCUMENTPATH:
			if (lexrc.next()) {
				document_path = ExpandPath(lexrc.GetString());
			}
			break;
			
		case RC_TEMPLATEPATH:
			if (lexrc.next())
				template_path = ExpandPath(lexrc.GetString());
			break;
			
		case RC_TEMPDIRPATH:
			if (lexrc.next())
				tempdir_path = ExpandPath(lexrc.GetString());
			break;
			
		case RC_USETEMPDIR:
			if (lexrc.next())
				use_tempdir = lexrc.GetBool();
			break;
			
		case RC_LASTFILES:
			if (lexrc.next())
				lastfiles = ExpandPath(lexrc.GetString());
			break;
			
                case RC_NUMLASTFILES:
                        if (lexrc.next())
                                num_lastfiles = lexrc.GetInteger();
			break;
			
                case RC_CHECKLASTFILES:
                        if (lexrc.next())
                                check_lastfiles = lexrc.GetBool();
			break;
			
		case RC_SCREEN_FONT_ROMAN:
			if (lexrc.next())
				roman_font_name = lexrc.GetString();
			break;
			
		case RC_SCREEN_FONT_SANS:
			if (lexrc.next())
				sans_font_name = lexrc.GetString();
			break;
			
		case RC_SCREEN_FONT_TYPEWRITER:
			if (lexrc.next())
				typewriter_font_name = lexrc.GetString();
			break;
			
		case RC_SCREEN_FONT_MENU:
			if (lexrc.next())
				menu_font_name = lexrc.GetString();
			break;
			
		case RC_SCREEN_FONT_POPUP:
			if (lexrc.next())
				popup_font_name = lexrc.GetString();
			break;
			
		case RC_SCREEN_FONT_ENCODING:
			if (lexrc.next()) {
				font_norm = lexrc.GetString();
				set_font_norm_type();
			}
			break;

		case RC_SCREEN_FONT_ENCODING_MENU:
			if (lexrc.next())
				font_norm_menu = lexrc.GetString();
			break;

		case RC_SET_COLOR:
		{
			string lyx_name, x11_name;

			if (lexrc.next())  {
				lyx_name = lexrc.GetString();
			} else {
				lexrc.printError("Missing color tag.");
				break;
			}
			
			if (lexrc.next()) {
				x11_name = lexrc.GetString();
			} else {
				lexrc.printError("Missing color name for color : `$$Token'");
				break;
			}

			if (!lcolor.setColor(lyx_name, x11_name))
				lyxerr << "Bad lyxrc set_color for "
					<< lyx_name << endl;

			break;
		}
		case RC_AUTOREGIONDELETE:
			// Auto region delete defaults to true
		        if (lexrc.next())
		        	auto_region_delete = lexrc.GetBool();
			break;
			
		case RC_BIND:
		{
			// we should not do an explicit binding before
			// loading a bind file. So, in this case, load
			// the default bind file.
			if (!hasBindFile) {
				ReadBindFile();
				bind_file = bindFile;
			}
			// !!!chb, dynamic key binding...
			int action, res = 0;
			string seq, cmd;
			
			if (lexrc.lex() == LyXLex::LEX_DATA)  {
				seq = lexrc.GetString();
			} else {
				lexrc.printError("Bad key sequence: `$$Token'");
				break;
			}
			
			if (lexrc.lex() == LyXLex::LEX_DATA) {
				cmd = lexrc.GetString();
			} else {
				lexrc.printError("Bad command: `$$Token'");
				break;
			}
			
			if ((action = lyxaction.LookupFunc(cmd))>= 0) {
				if (lyxerr.debugging(Debug::KBMAP)) {
					lyxerr << "RC_BIND: Sequence `"
					       << seq << "' Command `"
					       << cmd << "' Action `"
					       << action << '\'' << endl;
				}
				res = toplevel_keymap->bind(seq, action);
				if (res != 0) {
					lexrc.printError(
						"Invalid key sequence `"
						+ seq + '\''); 
				}
			} else {// cmd is the last token read.
				lexrc.printError(
					"Unknown LyX function `$$Token'");
			}
			break;
		}
		case RC_OVERRIDE_X_DEADKEYS:
			if (lexrc.next())
				override_x_deadkeys = lexrc.GetBool();
			break;

		case RC_SERVERPIPE:
			if (lexrc.next())
				lyxpipes = ExpandPath(lexrc.GetString());
			break;
			
		case RC_CURSOR_FOLLOWS_SCROLLBAR:
			if (lexrc.next())
				cursor_follows_scrollbar = lexrc.GetBool();
			break;

		case RC_FAX_COMMAND:
 			if (lexrc.next())
 				fax_command = lexrc.GetString();
			break;
		case RC_FAXPROGRAM:
			if (lexrc.next())
				fax_program = lexrc.GetString();
			break;
		case RC_PHONEBOOK:
			if (lexrc.next()) {
				string s = lexrc.GetString();
				if (AbsolutePath(s))
					phone_book = s;
				else
                                	phone_book = user_lyxdir + s;
			}
 			break;
		case RC_ASCIIROFF_COMMAND:
 			if (lexrc.next())
				ascii_roff_command = lexrc.GetString();
			break;
		case RC_ASCII_LINELEN:
			if (lexrc.next())
				ascii_linelen = lexrc.GetInteger();
			break;
			// Spellchecker settings:
		case RC_SPELL_COMMAND:
			if (lexrc.next())
				isp_command = lexrc.GetString();
			break;
		case RC_ACCEPT_COMPOUND:
			if (lexrc.next())
				isp_accept_compound = lexrc.GetBool();
			break;
		case RC_USE_INP_ENC:
			if (lexrc.next())
				isp_use_input_encoding = lexrc.GetBool();
			break;
		case RC_USE_ALT_LANG:
			if (lexrc.next())
				isp_use_alt_lang = lexrc.GetBool();
			break;
		case RC_USE_PERS_DICT:
			if (lexrc.next())
				isp_use_pers_dict = lexrc.GetBool();
			break;
		case RC_USE_ESC_CHARS:
			if (lexrc.next())
				isp_use_esc_chars = lexrc.GetBool();
			break;
		case RC_ALT_LANG:
			if (lexrc.next())
				isp_alt_lang = lexrc.GetString();
			break;
		case RC_PERS_DICT:
			if (lexrc.next())
				isp_pers_dict = lexrc.GetString();
			break;
		case RC_ESC_CHARS:
			if (lexrc.next())
				isp_esc_chars = lexrc.GetString();
			break;
		case RC_MAKE_BACKUP:
		        if (lexrc.next())
		                make_backup = lexrc.GetBool();
			break;
		case RC_BACKUPDIR_PATH:
			if (lexrc.next())
				backupdir_path = ExpandPath(lexrc.GetString());
			break;
		case RC_DATE_INSERT_FORMAT:
			if (lexrc.next())
				date_insert_format = lexrc.GetString();
			break;
		case RC_LANGUAGE_PACKAGE:
			if (lexrc.next())
				language_package = lexrc.GetString();
			break;
		case RC_LANGUAGE_AUTO_BEGIN:
			if (lexrc.next())
				language_auto_begin = lexrc.GetBool();
			break;
		case RC_LANGUAGE_AUTO_END:
			if (lexrc.next())
				language_auto_end = lexrc.GetBool();
			break;
		case RC_LANGUAGE_COMMAND_BEGIN:
			if (lexrc.next())
				language_command_begin = lexrc.GetString();
			break;
		case RC_LANGUAGE_COMMAND_END:
			if (lexrc.next())
				language_command_end = lexrc.GetString();
			break;
		case RC_RTL_SUPPORT:
			if (lexrc.next())
				rtl_support = lexrc.GetBool();
			break;
		case RC_AUTO_NUMBER:
			if (lexrc.next())
				auto_number = lexrc.GetBool();
			break;
		case RC_MARK_FOREIGN_LANGUAGE:
			if (lexrc.next())
				mark_foreign_language = lexrc.GetBool();
			break;
		case RC_SHOW_BANNER:
			if (lexrc.next())
				show_banner = lexrc.GetBool();
			break;
			
		case RC_NEW_ASK_FILENAME:
			if ( lexrc.next())
				new_ask_filename = lexrc.GetBool();
			break;
		case RC_CONVERTER: {
			string from, to, command, flags;
			if (lexrc.next())
				from = lexrc.GetString();
			if (lexrc.next())
				to = lexrc.GetString();
			if (lexrc.next())
				command = lexrc.GetString();
			if (lexrc.next())
				flags = lexrc.GetString();
			Converter::Add(from, to, command, flags);
			break;
		}
		case RC_VIEWER: {
			string format, command;
			if (lexrc.next())
				format = lexrc.GetString();
			if (lexrc.next())
				command = lexrc.GetString();
			Formats::SetViewer(format, command);
			break;
		}
		case RC_FORMAT: {
			string format, extension, prettyname, shortcut;
			if (lexrc.next())
				format = lexrc.GetString();
			if (lexrc.next())
				extension = lexrc.GetString();
			if (lexrc.next())
				prettyname = lexrc.GetString();
			if (lexrc.next())
				shortcut = lexrc.GetString();
			Formats::Add(format, extension, prettyname, shortcut);
			break;
		}
		case RC_DEFAULT_LANGUAGE:
			if ( lexrc.next())
				default_language = lexrc.GetString();
			break;

		case RC_LAST: break; // this is just a dummy
		}
	}

	return 0;
}


void LyXRC::write(string const & filename) const
{
	ofstream ofs(filename.c_str());
	if (ofs)
		output(ofs);
}


void LyXRC::print() const
{
	if (lyxerr.debugging())
		output(lyxerr);
	else
		output(cout);
}


void LyXRC::output(ostream & os) const
{
	os << "### This file is part of\n"
	   << "### ========================================================\n"
	   << "###          LyX, The Document Processor\n"
	   << "###\n"
	   << "###          Copyright 1995 Matthias Ettrich\n"
	   << "###          Copyright 1995-2000 The LyX Team.\n"
	   << "###\n"
	   << "### ========================================================\n"
	   << "\n"
	   << "# This file is written by LyX, if you want to make your own\n"
	   << "# modifications you should do them from inside LyX and save\n"
	   << "\n";
	
	// Why the switch you might ask. It is a trick to ensure that all
	// the elements in the LyXRCTags enum is handled. As you can see
	// there are no breaks at all. So it is just a huge fall-through.
	// The nice thing is that we will get a warning from the compiler
	// if we forget an element.
	LyXRCTags tag = RC_LAST;
	switch(tag) {
	case RC_LAST:
	case RC_INPUT:
		// input/include files are not done here
	case RC_BIND:
		// bindings is not written to the preferences file.
	case RC_BINDFILE:
		if (bind_file != system_lyxrc.bind_file) {
			os << "\\bind_file " << bind_file << "\n";
		}
		//
		// Misc Section
		//
		os << "\n#\n"
		   << "# MISC SECTION ######################################\n"
		   << "#\n\n";
		
	case RC_SHOW_BANNER:
		if (show_banner != system_lyxrc.show_banner) {
			os << "# Set to false to inhibit the startup banner.\n"
			   << "\\show_banner " << tostr(show_banner) << "\n";
		}
		// bind files are not done here.
	case RC_UIFILE:
		if (ui_file != system_lyxrc.ui_file) {
			os << "\\ui_file \"" << ui_file << "\"\n";
		}
	case RC_AUTOREGIONDELETE:
		if (auto_region_delete != system_lyxrc.auto_region_delete) {
			os << "# Set to false to inhibit automatic replacement of\n"
			   << "# the current selection.\n"
			   << "\\auto_region_delete " << tostr(auto_region_delete)
			   << "\n";
		}
	case RC_AUTOSAVE:
		if (autosave != system_lyxrc.autosave) {
			os << "# The time interval between auto-saves in seconds.\n"
			   << "\\autosave " << autosave << "\n";
		}
	case RC_EXIT_CONFIRMATION:
		if (exit_confirmation != system_lyxrc.exit_confirmation) {
			os << "# Ask for confirmation before exit if there are\n"
			   << "# unsaved changed documents.\n"
			   << "\\exit_confirmation " << tostr(exit_confirmation)
			   << "\n";
		}
	case RC_DISPLAY_SHORTCUTS:
		if (display_shortcuts != system_lyxrc.display_shortcuts) {
			os << "# Display name of the last command executed,\n"
			   << "# with a list of short-cuts in the minibuffer.\n" 
			   << "\\display_shortcuts " << tostr(display_shortcuts)
			   << "\n";
		}
	case RC_VIEWDVI_PAPEROPTION:
		if (view_dvi_paper_option 
		    != system_lyxrc.view_dvi_paper_option) {
			os << "# Options used to specify paper size to the\n"
			   << "# view_dvi_command\n"
			   << "\\view_dvi_paper_option \""
			   << view_dvi_paper_option << "\"\n";
		}
	case RC_DEFAULT_PAPERSIZE:
		if (default_papersize != system_lyxrc.default_papersize) {
			os << "# The default papersize to use.\n"
			   << "\\default_papersize \"";
			switch (default_papersize) {
			case BufferParams::PAPER_USLETTER:
				os << "usletter"; break;
			case BufferParams::PAPER_LEGALPAPER:
				os << "legal"; break;
			case BufferParams::PAPER_EXECUTIVEPAPER:
				os << "executive"; break;
			case BufferParams::PAPER_A3PAPER:
				os << "a3"; break;
			case BufferParams::PAPER_A4PAPER:
				os << "a4"; break;
			case BufferParams::PAPER_A5PAPER:
				os << "a5"; break;
			case BufferParams::PAPER_B5PAPER:
				os << "b5"; break;
			case BufferParams::PAPER_DEFAULT: break;
			}
			os << "\"\n";
		}
	case RC_PS_COMMAND:
		if (ps_command != system_lyxrc.ps_command) {
			os << "# Program used for interpreting postscript.\n"
			   << "\\ps_command \"" << ps_command << "\"\n";
		}
	case RC_CHKTEX_COMMAND:
		if (chktex_command != system_lyxrc.chktex_command) {
			os << "\\chktex_command \"" << chktex_command << "\"\n";
		}
	case RC_KBMAP:
		if (use_kbmap != system_lyxrc.use_kbmap) {
			os << "\\kbmap " << tostr(use_kbmap) << "\n";
		}
	case RC_KBMAP_PRIMARY:
		if (primary_kbmap != system_lyxrc.primary_kbmap) {
			os << "\\kbmap_primary \"" << primary_kbmap << "\"\n";
		}
	case RC_KBMAP_SECONDARY:
		if (secondary_kbmap != system_lyxrc.secondary_kbmap) {
			os << "\\kbmap_secondary \"" << secondary_kbmap
			   << "\"\n";
		}
	case RC_SERVERPIPE:
		if (lyxpipes != system_lyxrc.lyxpipes) {
			os << "\\serverpipe \"" << lyxpipes << "\"\n";
		}
	case RC_DATE_INSERT_FORMAT:
		if (date_insert_format != system_lyxrc.date_insert_format) {
			os << "\\date_insert_format \"" << date_insert_format
			   << "\"\n";
		}
		
		os << "\n#\n"
		   << "# SCREEN & FONTS SECTION ############################\n"
		   << "#\n\n";
		
	case RC_SCREEN_DPI:
		if (dpi != system_lyxrc.dpi) {
			os << "\\screen_dpi " << dpi << "\n";
		}
	case RC_SCREEN_ZOOM:
		if (zoom != system_lyxrc.zoom) {
			os << "\\screen_zoom " << zoom << "\n";
		}
	case RC_WHEEL_JUMP:
		if (wheel_jump != system_lyxrc.wheel_jump) {
			os << "\\wheel_jump " << wheel_jump << "\n";
		}
	case RC_CURSOR_FOLLOWS_SCROLLBAR:
		if (cursor_follows_scrollbar
		    != system_lyxrc.cursor_follows_scrollbar) {
			os << "\\cursor_follows_scrollbar "
			   << tostr(cursor_follows_scrollbar) << "\n";
		}
	case RC_SCREEN_FONT_ROMAN:
		if (roman_font_name != system_lyxrc.roman_font_name) {
			os << "\\screen_font_roman \"" << roman_font_name
			   << "\"\n";
		}
	case RC_SCREEN_FONT_SANS:
		if (sans_font_name != system_lyxrc.sans_font_name) {
			os << "\\screen_font_sans \"" << sans_font_name
			   << "\"\n";
		}
	case RC_SCREEN_FONT_TYPEWRITER:
		if (typewriter_font_name != system_lyxrc.typewriter_font_name) {
			os << "\\screen_font_typewriter \""
			   << typewriter_font_name << "\"\n";
		}
	case RC_SCREEN_FONT_SCALABLE:
		if (use_scalable_fonts != system_lyxrc.use_scalable_fonts) {
			os << "\\screen_font_scalable "
			   << tostr(use_scalable_fonts)
			   << "\n";
		}
	case RC_SCREEN_FONT_ENCODING:
		if (font_norm != system_lyxrc.font_norm) {
			os << "\\screen_font_encoding \"" << font_norm
			   << "\"\n";
		}
	case RC_SCREEN_FONT_POPUP:
		if (popup_font_name != system_lyxrc.popup_font_name) {
			os << "\\screen_font_popup \"" << popup_font_name
			   << "\"\n";
		}
	case RC_SCREEN_FONT_MENU:
		if (menu_font_name != system_lyxrc.menu_font_name) {
			os << "\\screen_font_menu \"" << menu_font_name
			   << "\"\n";
		}
	case RC_SCREEN_FONT_SIZES:
		if (font_sizes[LyXFont::SIZE_TINY]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_TINY] ||
		    font_sizes[LyXFont::SIZE_SCRIPT]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] ||
		    font_sizes[LyXFont::SIZE_FOOTNOTE]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] ||
		    font_sizes[LyXFont::SIZE_SMALL]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_SMALL] ||
		    font_sizes[LyXFont::SIZE_NORMAL]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_NORMAL] ||
		    font_sizes[LyXFont::SIZE_LARGE]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_LARGE] ||
		    font_sizes[LyXFont::SIZE_LARGER]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_LARGER] ||
		    font_sizes[LyXFont::SIZE_LARGEST]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_LARGEST] ||
		    font_sizes[LyXFont::SIZE_HUGE]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_HUGE] ||
		    font_sizes[LyXFont::SIZE_HUGER]
		    != system_lyxrc.font_sizes[LyXFont::SIZE_HUGER]) {
			os.setf(ios::fixed);
			os.precision(2);
			os << "\\screen_font_sizes";
			os << " " << font_sizes[LyXFont::SIZE_TINY];
			os << " " << font_sizes[LyXFont::SIZE_SCRIPT];
			os << " " << font_sizes[LyXFont::SIZE_FOOTNOTE];
			os << " " << font_sizes[LyXFont::SIZE_SMALL];
			os << " " << font_sizes[LyXFont::SIZE_NORMAL];
			os << " " << font_sizes[LyXFont::SIZE_LARGE];
			os << " " << font_sizes[LyXFont::SIZE_LARGER];
			os << " " << font_sizes[LyXFont::SIZE_LARGEST];
			os << " " << font_sizes[LyXFont::SIZE_HUGE];
			os << " " << font_sizes[LyXFont::SIZE_HUGER];
			os << "\n";
		}
		
	//case RC_SET_COLOR:
		// color bindings not written to preference file.
		// And we want to be warned about that. (Lgb)
		
		os << "\n#\n"
		   << "# PRINTER SECTION ###################################\n"
		   << "#\n\n";
		
	case RC_PRINTER:
		if (printer != system_lyxrc.printer) {
			os << "\\printer \"" << printer << "\"\n";
		}
	case RC_PRINT_ADAPTOUTPUT:
		if (print_adapt_output != system_lyxrc.print_adapt_output) {
			os << "\\print_adapt_output "
			   << tostr(print_adapt_output)
			   << "\n";
		}
	case RC_PRINT_COMMAND:
		if (print_command != system_lyxrc.print_command) {
			os << "\\print_command \"" << print_command << "\"\n";
		}
	case RC_PRINTEXSTRAOPTIONS:
		if (print_extra_options != system_lyxrc.print_extra_options) {
			os << "\\print_extra_options \"" << print_extra_options
			   << "\"\n";
		}
	case RC_PRINTSPOOL_COMMAND:
		if (print_spool_command != system_lyxrc.print_spool_command) {
			os << "\\print_spool_command \"" << print_spool_command
			   << "\"\n";
		}
	case RC_PRINTSPOOL_PRINTERPREFIX:
		if (print_spool_printerprefix
		    != system_lyxrc.print_spool_printerprefix) {
			os << "\\print_spool_printerprefix \""
			   << print_spool_printerprefix << "\"\n";
		}
	case RC_PRINTEVENPAGEFLAG:
		if (print_evenpage_flag != system_lyxrc.print_evenpage_flag) {
			os << "\\print_evenpage_flag \"" << print_evenpage_flag
			   << "\"\n";
		}
	case RC_PRINTODDPAGEFLAG:
		if (print_oddpage_flag != system_lyxrc.print_oddpage_flag) {
			os << "\\print_oddpage_flag \"" << print_oddpage_flag
			   << "\"\n";
		}
	case RC_PRINTREVERSEFLAG:
		if (print_reverse_flag != system_lyxrc.print_reverse_flag) {
			os << "\\print_reverse_flag \"" << print_reverse_flag
			   << "\"\n";
		}
	case RC_PRINTLANDSCAPEFLAG:
		if (print_landscape_flag != system_lyxrc.print_landscape_flag) {
			os << "\\print_landscape_flag \"" << print_landscape_flag
			   << "\"\n";
		}
	case RC_PRINTPAGERANGEFLAG:
		if (print_pagerange_flag != system_lyxrc.print_pagerange_flag) {
			os << "\\print_pagerange_flag \"" << print_pagerange_flag
			   << "\"\n";
		}
	case RC_PRINTCOPIESFLAG:
		if (print_copies_flag != system_lyxrc.print_copies_flag) {
			os << "\\print_copies_flag \"" << print_copies_flag 
			   << "\"\n";
		}
	case RC_PRINTCOLLCOPIESFLAG:
		if (print_collcopies_flag
		    != system_lyxrc.print_collcopies_flag) {
			os << "\\print_collcopies_flag \""
			   << print_collcopies_flag
			   << "\"\n";
		}
	case RC_PRINTPAPERFLAG:
		if (print_paper_flag != system_lyxrc.print_paper_flag) {
			os << "\\print_paper_flag \"" << print_paper_flag
			   << "\"\n";
		}
	case RC_PRINTPAPERDIMENSIONFLAG:
		if (print_paper_dimension_flag
		    != system_lyxrc.print_paper_dimension_flag) {
			os << "\\print_paper_dimension_flag \""
			   << print_paper_dimension_flag << "\"\n";
		}
	case RC_PRINTTOPRINTER:
		if (print_to_printer != system_lyxrc.print_to_printer) {
			os << "\\print_to_printer \"" << print_to_printer
			   << "\"\n";
		}
	case RC_PRINTTOFILE:
		if (print_to_file != system_lyxrc.print_to_file) {
			os << "\\print_to_file \"" << print_to_file << "\"\n";
		}
	case RC_PRINTFILEEXTENSION:
		if (print_file_extension != system_lyxrc.print_file_extension) {
			os << "\\print_file_extension \""
			   << print_file_extension
			   << "\"\n";
		}

		os << "\n#\n"
		   << "# EXPORT SECTION ####################################\n"
		   << "#\n\n";
		
	case RC_CUSTOM_EXPORT_COMMAND:
		if (custom_export_command
		    != system_lyxrc.custom_export_command) {
			os << "\\custom_export_command \""
			   << custom_export_command
			   << "\"\n";
		}
	case RC_CUSTOM_EXPORT_FORMAT:
		if (custom_export_format
		    != system_lyxrc.custom_export_format) {
			os << "\\custom_export_format \"" << custom_export_format
			   << "\"\n";
		}

		os << "\n#\n"
		   << "# TEX SECTION #######################################\n"
		   << "#\n\n";
		
	case RC_FONT_ENCODING:
		if (fontenc != system_lyxrc.fontenc) {
			os << "\\font_encoding \"" << fontenc << "\"\n";
		}

		os << "\n#\n"
		   << "# FILE SECTION ######################################\n"
		   << "#\n\n";

	case RC_DOCUMENTPATH:
		if (document_path != system_lyxrc.document_path) {
			os << "\\document_path \"" << document_path << "\"\n";
		}
	case RC_LASTFILES:
		if (lastfiles != system_lyxrc.lastfiles) {
			os << "\\lastfiles \"" << lastfiles << "\"\n";
		}
	case RC_NUMLASTFILES:
		if (num_lastfiles != system_lyxrc.num_lastfiles) {
			os << "\\num_lastfiles " << num_lastfiles << "\n";
		}
	case RC_CHECKLASTFILES:
		if (check_lastfiles != system_lyxrc.check_lastfiles) {
			os << "\\check_lastfiles " << tostr(check_lastfiles)
			   << "\n";
		}
	case RC_TEMPLATEPATH:
		if (template_path != system_lyxrc.template_path) {
			os << "\\template_path \"" << template_path << "\"\n";
		}
	case RC_TEMPDIRPATH:
		if (tempdir_path != system_lyxrc.tempdir_path) {
			os << "\\tempdir_path \"" << tempdir_path << "\"\n";
		}
	case RC_USETEMPDIR:
		if (use_tempdir != system_lyxrc.use_tempdir) {
			os << "\\use_tempdir " << tostr(use_tempdir) << "\n";
		}
	case RC_ASCII_LINELEN:
		if (ascii_linelen != system_lyxrc.ascii_linelen) {
			os << "\\ascii_linelen " << ascii_linelen << "\n";
		}
	case RC_MAKE_BACKUP:
		if (make_backup != system_lyxrc.make_backup) {
			os << "\\make_backup " << tostr(make_backup) << "\n";
		}
	case RC_BACKUPDIR_PATH:
		if (backupdir_path != system_lyxrc.backupdir_path) {
			os << "\\backupdir_path \"" << backupdir_path << "\"\n";
		}

		os << "\n#\n"
		   << "# FAX SECTION #######################################\n"
		   << "#\n\n";
		
	case RC_FAX_COMMAND:
		if (fax_command != system_lyxrc.fax_command) {
			os << "\\fax_command \"" << fax_command << "\"\n";
		}
	case RC_PHONEBOOK:
		if (phone_book != system_lyxrc.phone_book) {
			os << "\\phone_book \"" << phone_book << "\"\n";
		}
	case RC_FAXPROGRAM:
		if (fax_program != system_lyxrc.fax_program) {
			os << "\\fax_program \"" << fax_program << "\"\n";
		}

		os << "\n#\n"
		   << "# ASCII EXPORT SECTION ##############################\n"
		   << "#\n\n";

	case RC_ASCIIROFF_COMMAND:
		if (ascii_roff_command != system_lyxrc.ascii_roff_command) {
			os << "\\ascii_roff_command \"" << ascii_roff_command
			   << "\"\n";
		}

		os << "\n#\n"
		   << "# SPELLCHECKER SECTION ##############################\n"
		   << "#\n\n";

	case RC_SPELL_COMMAND:
		if (isp_command != system_lyxrc.isp_command) {
			os << "\\spell_command \"" << isp_command << "\"\n";
		}
	case RC_ACCEPT_COMPOUND:
		if (isp_accept_compound != system_lyxrc.isp_accept_compound) {
			os << "\\accept_compound " << tostr(isp_accept_compound)
			   << "\n";
		}
	case RC_USE_ALT_LANG:
		if (isp_use_alt_lang != system_lyxrc.isp_use_alt_lang) {
			os << "\\use_alt_language " << tostr(isp_use_alt_lang)
			   << "\n";
		}
	case RC_ALT_LANG:
		if (isp_alt_lang != system_lyxrc.isp_alt_lang) {
			os << "\\alternate_language \"" << isp_alt_lang
			   << "\"\n";
		}
	case RC_USE_ESC_CHARS:
		if (isp_use_esc_chars != system_lyxrc.isp_use_esc_chars) {
			os << "\\use_escape_chars " << tostr(isp_use_esc_chars)
			   << "\n";
		}
	case RC_ESC_CHARS:
		if (isp_esc_chars != system_lyxrc.isp_esc_chars) {
			os << "\\escape_chars \"" << isp_esc_chars << "\"\n";
		}
	case RC_USE_PERS_DICT:
		if (isp_use_pers_dict != system_lyxrc.isp_use_pers_dict) {
			os << "\\use_personal_dictionary "
			   << tostr(isp_use_pers_dict)
			   << "\n";
		}
	case RC_PERS_DICT:
		if (isp_pers_dict != system_lyxrc.isp_pers_dict) {
			os << "\\personal_dictionary \"" << isp_pers_dict
			   << "\"\n";
		}
	case RC_USE_INP_ENC:
		if (isp_use_input_encoding
		    != system_lyxrc.isp_use_input_encoding) {
			os << "\\use_input_encoding "
			   << tostr(isp_use_input_encoding)
			   << "\n";
		}

		os << "\n#\n"
		   << "# LANGUAGE SUPPORT SECTION ##########################\n"
		   << "#\n\n";

	case RC_RTL_SUPPORT:
		if (rtl_support != system_lyxrc.rtl_support) {
			os << "\\rtl " << tostr(rtl_support) << "\n";
		}
	case RC_LANGUAGE_PACKAGE:
		if (language_package != system_lyxrc.language_package) {
			os << "\\language_package \"" << language_package
			   << "\"\n";
		}
	case RC_LANGUAGE_COMMAND_BEGIN:
		if (language_command_begin
		    != system_lyxrc.language_command_begin) {
			os << "\\language_command_begin \""
			   << language_command_begin
			   << "\"\n";
		}
	case RC_LANGUAGE_COMMAND_END:
		if (language_command_end
		    != system_lyxrc.language_command_end) {
			os << "\\language_command_end \"" << language_command_end
			   << "\"\n";
		}
	case RC_LANGUAGE_AUTO_BEGIN:
		if (language_auto_begin != system_lyxrc.language_auto_begin) {
			os << "\\language_auto_begin " 
			   << tostr(language_auto_begin) << "\n";
		}
	case RC_LANGUAGE_AUTO_END:
		if (language_auto_end != system_lyxrc.language_auto_end) {
			os << "\\language_auto_end " 
			   << tostr(language_auto_end) << "\n";
		}
	case RC_MARK_FOREIGN_LANGUAGE:
		if (mark_foreign_language
		    != system_lyxrc.mark_foreign_language) {
			os << "\\mark_foreign_language " <<
				tostr(mark_foreign_language) << "\n";
		}

		os << "\n#\n"
		   << "# 2nd MISC SUPPORT SECTION ##########################\n"
		   << "#\n\n";

	case RC_OVERRIDE_X_DEADKEYS:
		if (override_x_deadkeys != system_lyxrc.override_x_deadkeys) {
			os << "\\override_x_deadkeys "
			   << tostr(override_x_deadkeys) << "\n";
		}
	case RC_SCREEN_FONT_ENCODING_MENU:
		if (font_norm_menu != system_lyxrc.font_norm_menu) {
			os << "\\screen_font_encoding_menu \"" << font_norm_menu
			   << "\"\n";
		}
	case RC_AUTO_NUMBER:
		if (auto_number != system_lyxrc.auto_number) {
			os << "\\auto_number " << tostr(auto_number) << "\n";
		}
	case RC_NEW_ASK_FILENAME:
		if (new_ask_filename != system_lyxrc.new_ask_filename) {
			os << "\\new_ask_filename " << tostr(new_ask_filename)
			   << "\n";
		}
	case RC_DEFAULT_LANGUAGE:
		if (default_language != system_lyxrc.default_language) {
			os << "\\default_language " << default_language << "\n";
		}
	}
	os.flush();
}

void LyXRC::set_font_norm_type()
{
	if (font_norm == "iso10646-1")
		font_norm_type = ISO_10646_1;
	else if (font_norm == "iso8859-1")
		font_norm_type = ISO_8859_1;
	else if (font_norm == "iso8859-6.8x")
		font_norm_type = ISO_8859_6_8;
	else if (font_norm == "iso8859-9")
		font_norm_type = ISO_8859_9;
	else
		font_norm_type = OTHER_ENCODING;
}

string LyXRC::getFeedback(LyXRCTags tag)
{
	string str;
	
	switch( tag ) {
	case RC_FONT_ENCODING:
		str = N_("The font encoding used for the LaTeX2e fontenc package.\nT1 is highly recommended for non-English languages.");
		break;
		
	case RC_PRINTER:
		str = N_("The default printer to print on. If none is specified, LyX will\nuse the environment variable PRINTER.");
		break;
		
	case RC_PRINT_COMMAND:
		str = N_("Your favorite print program, eg \"dvips\", \"dvilj4\"");
		break;
		
	case RC_PRINTEVENPAGEFLAG:
	case RC_PRINTODDPAGEFLAG:
	case RC_PRINTPAGERANGEFLAG:
	case RC_PRINTCOPIESFLAG:
	case RC_PRINTCOLLCOPIESFLAG:
	case RC_PRINTREVERSEFLAG:
	case RC_PRINTLANDSCAPEFLAG:
        case RC_PRINTPAPERFLAG:
	case RC_PRINTPAPERDIMENSIONFLAG:
		str = N_("Look at the man page for your favorite print program to learn\nwhich options to use.");
		break;
		
	case RC_PRINTTOPRINTER:
		str = N_("Option to pass to the print program to print on a specific printer.");
		break;
		
	case RC_PRINT_ADAPTOUTPUT:
		str = N_("Set to true for LyX to pass the name of the destination printer to your\nprint command.");
		break;
		
	case RC_PRINTTOFILE:
		str = N_("Option to pass to the print program to print to a file.");
		break;
		
	case RC_PRINTFILEEXTENSION:
		str = N_("Extension of printer program output file. Usually .ps");
		break;
		
	case RC_PRINTEXSTRAOPTIONS:
		str = N_("Extra options to pass to printing program after everything else,\nbut before the filename of the DVI file to be printed.");
		break;
		
	case RC_PRINTSPOOL_COMMAND:
		str = N_("When set, this printer option automatically prints to a file and then calls\na separate print spooling program on that file with the given name\nand arguments.");
		break;
		
	case RC_PRINTSPOOL_PRINTERPREFIX:
		str = N_("If you specify a printer name in the print dialog, the following argument\nis prepended along with the printer name after the spool command.");
		break;
		
	case RC_SCREEN_DPI:
		str = N_("DPI (dots per inch) of your monitor is auto-detected by LyX.\nIf that goes wrong, override the setting here.");
		break;
		
	case RC_SCREEN_ZOOM:
		str = N_("The zoom percentage for screen fonts.\nA setting of 100% will make the fonts roughly the same size as on paper.");
		break;
		
	case RC_SCREEN_FONT_SIZES:
		str = N_("The font sizes used for calculating the scaling of the screen fonts.");
		break;
		
	case RC_SCREEN_FONT_ROMAN:
	case RC_SCREEN_FONT_SANS:
	case RC_SCREEN_FONT_TYPEWRITER:
		str = N_("The screen fonts used to display the text while editing.");
		break;
		
	case RC_SCREEN_FONT_MENU:
		str = N_("The font for menus (and groups titles in popups).");
		break;
		
	case RC_SCREEN_FONT_POPUP:
		str = N_("The font for popups.");
		break;
		
	case RC_SCREEN_FONT_ENCODING:
		str = N_("The norm for the screen fonts.");
		break;
		
	case RC_SCREEN_FONT_ENCODING_MENU:
		str = N_("The norm for the menu/popups fonts.");
		break;
		
	case RC_SET_COLOR:
		break;
		
	case RC_AUTOSAVE:
		str = N_("The time interval between auto-saves (in seconds).\n0 means no auto-save");
		break;
		
	case RC_DOCUMENTPATH:
		str = N_("The default path for your documents.");
		break;
		
	case RC_TEMPLATEPATH:
		str = N_("The path that LyX will set when offering to choose a template.");
		break;
		
	case RC_TEMPDIRPATH:
		str = N_("The path that LyX will use to store temporary TeX output.");
		break;
		
	case RC_USETEMPDIR:
		str = N_("Specify to use a temporary directory to store temporary TeX output.\nThis directory is deleted when you quit LyX.");
		break;
		
	case RC_LASTFILES:
		str = N_("The file where the last-files information should be stored.");
		break;
		
	case RC_AUTOREGIONDELETE:
		str = N_("Set to false if you don't want the current selection to be replaced\nautomatically by what you type.");
		break;
		
	case RC_OVERRIDE_X_DEADKEYS:
		str = N_("Set to true for LyX to take over the handling of the dead keys\n(a.k.a accent keys) that may be defined for your keyboard.");
		break;
		

	case RC_SERVERPIPE:
		str = N_("This starts the lyxserver. The pipes get an additional extension\n\".in\" and \".out\". Only for advanced users.");
		break;
		
	case RC_BINDFILE:
		str = N_("Keybindings file. Can either specify an absolute path,\nor LyX will look in its global and local bind/ directories.");
		break;
		
	case RC_UIFILE:
		str = N_("The  UI (user interface) file. Can either specify an absolute path,\nor LyX will look in its global and local ui/ directories.");
		break;
		
	case RC_KBMAP:
	case RC_KBMAP_PRIMARY:
	case RC_KBMAP_SECONDARY:
		str = N_("Use this to set the correct mapping file for your keyboard.\nYou'll need this if you for instance want to type German documents\non an American keyboard.");
		break;
		
	case RC_FAX_COMMAND:
		break;
		
	case RC_PHONEBOOK:
		break;
		
	case RC_FAXPROGRAM:
		break;
		
	case RC_ASCIIROFF_COMMAND:
		str = N_("Use to define an external program to render tables in the ASCII output.\nE.g. \"groff -t -Tlatin1 $$FName\"  where $$FName is the input file.\nIf \"none\" is specified, an internal routine is used.");
		break;
		
	case RC_ASCII_LINELEN:
		str = N_("This is the maximum line length of an exported ASCII file\n(LaTeX, SGML or plain text).");
		break;
		
	case RC_NUMLASTFILES:
		str = N_("Maximal number of lastfiles. Up to 9 can appear in the file menu.");
		break;
		
	case RC_CHECKLASTFILES:
		str = N_("Specify to check whether the lastfiles still exist.");
		break;
		
	case RC_VIEWDVI_PAPEROPTION:
		break;
		
	case RC_DEFAULT_PAPERSIZE:
		str = N_("Specify the default paper size.");
		break;
		
	case RC_PS_COMMAND:
		break;
		
	case RC_ACCEPT_COMPOUND:
		str = N_("Consider run-together words, such as \"notthe\" for \"not the\",\nas legal words?");
		break;
		
	case RC_SPELL_COMMAND:
		str = N_("What command runs the spell checker?");
		break;
		
	case RC_USE_INP_ENC:
		str = N_("Specify whether to pass the -T input encoding option to ispell.\nEnable this if you can't spellcheck words with international letters\nin them.\nThis may not work with all dictionaries.");
		break;
		
	case RC_USE_ALT_LANG:
	case RC_ALT_LANG:
		str = N_("Specify an alternate language.\nThe default is to use the language of the document.");
		break;
		
	case RC_USE_PERS_DICT:
	case RC_PERS_DICT:
		str = N_("Specify an alternate personal dictionary file.\nE.g. \".ispell_english\".");
		break;
		
	case RC_USE_ESC_CHARS:
	case RC_ESC_CHARS:
		str = N_("Specify additional chars that can be part of a word.");
		break;
		
	case RC_SCREEN_FONT_SCALABLE:
		str = N_("Allow the use of scalable screen fonts? If false, LyX will use the\nclosest existing size for a match. Use this if the scalable fonts\nlook bad and you have many fixed size fonts.");
		break;
		
	case RC_CHKTEX_COMMAND:
		str = N_("Define how to run chktex.\nE.g. \"chktex -n11 -n1 -n3 -n6 -n9 -22 -n25 -n30 -n38\"\nRefer to the ChkTeX documentation.");
		break;
		
	case RC_CURSOR_FOLLOWS_SCROLLBAR:
		str = N_("LyX normally doesn't update the cursor position if you move the scrollbar.\nSet to true if you'd prefer to always have the cursor on screen.");
		break;
		
	case RC_EXIT_CONFIRMATION:
		str = N_("Sets whether LyX asks for a second confirmation to exit when you have\nchanged documents.\n(LyX will still ask to save changed documents.)");
		break;
		
	case RC_DISPLAY_SHORTCUTS:
		str = N_("LyX continously displays names of last command executed,\nalong with a list of defined short-cuts for it in the minibuffer.\nSet to false if LyX seems slow.");
		break;
		
	case RC_MAKE_BACKUP:
		str = N_("Set to false if you don't want LyX to create backup files.");
		break;
		
	case RC_BACKUPDIR_PATH:
		str = N_("The path for storing backup files. If it is an empty string,\nLyX will store the backup file in the same directory as the original file.");
		break;

	case RC_RTL_SUPPORT:
		str = N_("Use to enable support of right-to-left languages (e.g. Hebrew, Arabic).");
		break;
		
	case RC_MARK_FOREIGN_LANGUAGE:
		str = N_("Use to control the highlighting of words with a language foreign to\nthat of the document.");
		break;
		
	case RC_LANGUAGE_PACKAGE:
		str = N_("The latex command for loading the language package.\nE.g. \"\\usepackage{babel}\", \"\\usepackage{omega}\".");
		break;
		
	case RC_LANGUAGE_AUTO_BEGIN:
		str = N_("Use if a language switching command is needed at the beginning\nof the document.");
		break;
		
	case RC_LANGUAGE_AUTO_END:
		str = N_("Use if a language switching command is needed at the end\nof the document.");
		break;
		
	case RC_LANGUAGE_COMMAND_BEGIN:
		str = N_("The latex command for changing from the language of the document\nto another language.\nE.g. \\selectlanguage{$$lang} where $$lang is substituted by the name\nof the second language.");
		break;
		
	case RC_LANGUAGE_COMMAND_END:
		str = N_("The latex command for changing back to the language of the document.");
		break;
		
	case RC_DATE_INSERT_FORMAT:
		str = N_("This accepts the normal strftime formats; see man strftime for full details.\nE.g.\"%A, %e. %B %Y\".");
		break;
		
	case RC_SHOW_BANNER:
		str = N_("Set to false if you don't want the startup banner.");
		break;
		
	case RC_WHEEL_JUMP:
		str = N_("The wheel movement factor (for mice with wheels or five button mice)");
		break;
		
	case RC_CONVERTER:
		break;
		
	case RC_VIEWER:
		break;
		
	case RC_FORMAT:
		break;
		
	case RC_NEW_ASK_FILENAME:
		str = N_("This sets the behaviour if you want to be asked for a filename when\ncreating a new document or wait until you save it and be asked then.");
		break;
		
	case RC_DEFAULT_LANGUAGE:
		str = N_("New documents will be assigned this language.");
		break;
		
	default:
		break;
	}

	return str;
}

// The global instance
LyXRC lyxrc;

// The global copy of the system lyxrc entries (everything except preferences)
LyXRC system_lyxrc;
