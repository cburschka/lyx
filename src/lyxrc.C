/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxrc.h"
#endif

#include "debug.h"

#include "lyxrc.h"
#include "LyXAction.h"
#include "lyxserver.h"
#include "lyx_main.h"
#include "intl.h"
#include "tex-strings.h"
#include "support/path.h"
#include "support/filetools.h"
#include "lyxtext.h"

// this is crappy... why are those colors command line arguments and
// not in lyxrc?? (Matthias) 
// Because nobody put them there. (Asger)
extern int fast_selection;
extern string background_color;
extern char selection_color[];
extern bool cursor_follows_scrollbar;
extern kb_keymap *toplevel_keymap;
extern LyXAction lyxaction;

enum _LyXRCTags {
	RC_BEGINTOOLBAR=1,
	RC_FONT_ENCODING,
	RC_PRINTER,
	RC_PRINT_COMMAND,
	RC_PRINTEVENPAGEFLAG,
	RC_PRINTODDPAGEFLAG,
	RC_PRINTPAGERANGEFLAG,
	RC_PRINTCOPIESFLAG,
	RC_PRINTCOLLCOPIESFLAG,
	RC_PRINTREVERSEFLAG,
	RC_PRINTLANDSCAPEFLAG,
	RC_PRINTTOPRINTER,
	RC_PRINT_ADAPTOUTPUT,
	RC_PRINTTOFILE,
	RC_PRINTFILEEXTENSION,
	RC_PRINTEXSTRAOPTIONS,
	RC_PRINTSPOOL_COMMAND,
	RC_PRINTSPOOL_PRINTERPREFIX,
        RC_PRINTPAPERFLAG,
	RC_PRINTPAPERDIMENSIONFLAG,
	RC_CUSTOM_EXPORT_COMMAND,
	RC_CUSTOM_EXPORT_FORMAT,
	RC_LATEX_COMMAND,
        RC_LITERATE_COMMAND,
        RC_LITERATE_EXTENSION,
        RC_LITERATE_ERROR_FILTER,
        RC_BUILD_COMMAND,
        RC_BUILD_ERROR_FILTER,
	RC_SCREEN_DPI,
	RC_SCREEN_ZOOM,
	RC_SCREEN_FONT_SIZES,
	RC_SCREEN_FONT_ROMAN,
	RC_SCREEN_FONT_SANS,
	RC_SCREEN_FONT_TYPEWRITER,
	RC_SCREEN_FONT_MENU,
	RC_SCREEN_FONT_POPUP,
	RC_SCREEN_FONT_ENCODING,
	RC_AUTOSAVE,
	RC_SGML_EXTRA_OPTIONS,
	RC_DOCUMENTPATH,
	RC_TEMPLATEPATH,
	RC_TEMPDIRPATH,
	RC_USETEMPDIR,
	RC_LASTFILES,
	RC_AUTOREGIONDELETE,
	RC_BIND,
	RC_SERVERPIPE,
	RC_NOMENUACCELERATORS,
	RC_INPUT,
	RC_BINDFILE,
	RC_KBMAP,
	RC_KBMAP_PRIMARY,
	RC_KBMAP_SECONDARY,
	RC_FAST_SELECTION,
	RC_SELECTION_COLOR,
	RC_BACKGROUND_COLOR,
	RC_FAX_COMMAND,
	RC_PHONEBOOK,
	RC_FAXPROGRAM,
	RC_ASCIIROFF_COMMAND,
	RC_ASCII_LINELEN,
	RC_NUMLASTFILES,
	RC_CHECKLASTFILES,
	RC_VIEWDVI_COMMAND,
	RC_DEFAULT_PAPERSIZE,
	RC_PS_COMMAND,
	RC_VIEWPS_COMMAND,
	RC_VIEWPSPIC_COMMAND,
	RC_ACCEPT_COMPOUND,
	RC_SPELL_COMMAND,
	RC_USE_INP_ENC,
	RC_USE_ALT_LANG,
	RC_USE_PERS_DICT,
	RC_USE_ESC_CHARS,
	RC_SCREEN_FONT_SCALABLE,
	RC_ALT_LANG,
	RC_PERS_DICT,
	RC_ESC_CHARS,
	RC_CHKTEX_COMMAND,
	RC_CURSOR_FOLLOWS_SCROLLBAR,
	RC_EXIT_CONFIRMATION,
	RC_DISPLAY_SHORTCUTS,
	RC_RELYX_COMMAND,
	RC_HTML_COMMAND,
	RC_LAST	
};

static keyword_item lyxrcTags[] = {
	{ "\\accept_compound", RC_ACCEPT_COMPOUND },
	{ "\\alternate_language", RC_ALT_LANG },
	{ "\\ascii_linelen", RC_ASCII_LINELEN },
	{ "\\ascii_roff_command", RC_ASCIIROFF_COMMAND },
	{ "\\auto_region_delete", RC_AUTOREGIONDELETE },
	{ "\\autosave", RC_AUTOSAVE },
	{ "\\background_color", RC_BACKGROUND_COLOR },
	{ "\\begin_toolbar", RC_BEGINTOOLBAR },
	{ "\\bind", RC_BIND },
	{ "\\bind_file", RC_BINDFILE },
        { "\\build_command", RC_BUILD_COMMAND },
        { "\\build_error_filter", RC_BUILD_ERROR_FILTER },
	{ "\\check_lastfiles", RC_CHECKLASTFILES },
	{ "\\chktex_command", RC_CHKTEX_COMMAND },
	{ "\\cursor_follows_scrollbar", RC_CURSOR_FOLLOWS_SCROLLBAR },
	{ "\\custom_export_command", RC_CUSTOM_EXPORT_COMMAND },
	{ "\\custom_export_format", RC_CUSTOM_EXPORT_FORMAT },
	{ "\\default_papersize", RC_DEFAULT_PAPERSIZE },
	{ "\\display_shortcuts", RC_DISPLAY_SHORTCUTS },
	{ "\\document_path", RC_DOCUMENTPATH },
	{ "\\escape_chars", RC_ESC_CHARS },
	{ "\\exit_confirmation", RC_EXIT_CONFIRMATION },
	{ "\\fast_selection", RC_FAST_SELECTION },
	{ "\\fax_command", RC_FAX_COMMAND },
	{ "\\fax_program", RC_FAXPROGRAM },
	{ "\\font_encoding", RC_FONT_ENCODING },
	{ "\\html_command", RC_HTML_COMMAND },
	{ "\\input", RC_INPUT },
	{ "\\kbmap", RC_KBMAP },
	{ "\\kbmap_primary", RC_KBMAP_PRIMARY },
	{ "\\kbmap_secondary", RC_KBMAP_SECONDARY },
	{ "\\lastfiles", RC_LASTFILES },
	{ "\\latex_command", RC_LATEX_COMMAND },
        { "\\literate_command", RC_LITERATE_COMMAND },
        { "\\literate_error_filter", RC_LITERATE_ERROR_FILTER },
        { "\\literate_extension", RC_LITERATE_EXTENSION },
	{ "\\num_lastfiles", RC_NUMLASTFILES },
	{ "\\personal_dictionary", RC_PERS_DICT },
	{ "\\phone_book", RC_PHONEBOOK },
	{ "\\print_adapt_output", RC_PRINT_ADAPTOUTPUT },
	{ "\\print_collcopies_flag", RC_PRINTCOLLCOPIESFLAG },
	{ "\\print_command", RC_PRINT_COMMAND },
	{ "\\print_copies_flag", RC_PRINTCOPIESFLAG },
	{ "\\print_evenpage_flag", RC_PRINTEVENPAGEFLAG },
	{ "\\print_extra_options", RC_PRINTEXSTRAOPTIONS },
	{ "\\print_file_extension", RC_PRINTFILEEXTENSION },
	{ "\\print_landscape_flag", RC_PRINTLANDSCAPEFLAG },
	{ "\\print_oddpage_flag", RC_PRINTODDPAGEFLAG },
	{ "\\print_pagerange_flag", RC_PRINTPAGERANGEFLAG },
        { "\\print_paper_dimension_flag", RC_PRINTPAPERDIMENSIONFLAG },
        { "\\print_paper_flag", RC_PRINTPAPERFLAG },
	{ "\\print_reverse_flag", RC_PRINTREVERSEFLAG },
	{ "\\print_spool_command", RC_PRINTSPOOL_COMMAND },
	{ "\\print_spool_printerprefix", RC_PRINTSPOOL_PRINTERPREFIX },
	{ "\\print_to_file", RC_PRINTTOFILE },
	{ "\\print_to_printer", RC_PRINTTOPRINTER },
	{ "\\printer", RC_PRINTER },
	{ "\\ps_command", RC_PS_COMMAND },
	{ "\\relyx_command", RC_RELYX_COMMAND },
	{ "\\screen_dpi", RC_SCREEN_DPI },
	{ "\\screen_font_encoding", RC_SCREEN_FONT_ENCODING },
	{ "\\screen_font_menu", RC_SCREEN_FONT_MENU },
	{ "\\screen_font_popup", RC_SCREEN_FONT_POPUP },
	{ "\\screen_font_roman", RC_SCREEN_FONT_ROMAN },
	{ "\\screen_font_sans", RC_SCREEN_FONT_SANS },
	{ "\\screen_font_scalable", RC_SCREEN_FONT_SCALABLE },
	{ "\\screen_font_sizes", RC_SCREEN_FONT_SIZES },
	{ "\\screen_font_typewriter", RC_SCREEN_FONT_TYPEWRITER },
	{ "\\screen_zoom", RC_SCREEN_ZOOM },
	{ "\\selection_color", RC_SELECTION_COLOR },
	{ "\\serverpipe", RC_SERVERPIPE },
	{ "\\sgml_extra_options", RC_SGML_EXTRA_OPTIONS },
	{ "\\spell_command", RC_SPELL_COMMAND },
	{ "\\tempdir_path", RC_TEMPDIRPATH },
	{ "\\template_path", RC_TEMPLATEPATH },
	{ "\\use_alt_language", RC_USE_ALT_LANG },
	{ "\\use_escape_chars", RC_USE_ESC_CHARS },
	{ "\\use_input_encoding", RC_USE_INP_ENC },
	{ "\\use_personal_dictionary", RC_USE_PERS_DICT },
	{ "\\use_tempdir", RC_USETEMPDIR },
	{ "\\view_dvi_command", RC_VIEWDVI_COMMAND },
	{ "\\view_ps_command", RC_VIEWPS_COMMAND },
        { "\\view_pspic_command", RC_VIEWPSPIC_COMMAND }
};

/* Let the range depend of the size of lyxrcTags.  Alejandro 240596 */
static const int lyxrcCount = sizeof(lyxrcTags) / sizeof(keyword_item);

// Should this be moved inside LyXAction? 
static inline
int bindKey(char const* seq, int action)
{ 
	return toplevel_keymap->bind(seq, action); 
}


LyXRC::LyXRC()
{
	// Get printer from the environment. If fail, use default "",
	// assuming that everything is set up correctly.
	printer = GetEnv("PRINTER");
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
	latex_command = "latex";
        literate_command = "none";
        literate_extension = "none";
        literate_error_filter = "cat";
        build_command = "make";
        build_error_filter = "cat";
	relyx_command = "reLyX";
	ps_command = "gs";
	view_ps_command = "ghostview -swap";
	view_pspic_command = "ghostview";
	view_dvi_command = "xdvi";
	default_papersize = PAPER_USLETTER;
	custom_export_format = "ps";
	chktex_command = "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38";
	html_command = "tth -t";
	fontenc = "default";
	dpi = 75;
	// Because a screen typically is wider than a piece of paper:
	zoom = 150;
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
	autosave = 300;
	auto_region_delete = true;
	ascii_linelen = 75;
	num_lastfiles = 4;
	check_lastfiles = true;
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
	defaultKeyBindings();
}


LyXRC::~LyXRC()
{
}

int LyXRC::ReadBindFile(string name)
{
	hasBindFile = true;
	string tmp = i18nLibFileSearch("bind",name,"bind");
	lyxerr[Debug::LYXRC] << "Reading bindfile:" << tmp << endl;
	int result = Read(tmp);
	if (result) {
		lyxerr << "Error reading bind file: " << tmp << endl;
	}
	return result;
}

int LyXRC::Read(string const &filename)
{
	// Default bindfile.
	string bindFile = "cua";

	LyXLex lexrc(lyxrcTags, lyxrcCount);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable();
	
	lexrc.setFile(filename);
	if (!lexrc.IsOK()) return -2;

	lyxerr[Debug::INIT] << "Reading '" << filename << "'..." << endl;
	
	while (lexrc.IsOK()) {

		switch(lexrc.lex()) {
		case LyXLex::LEX_FEOF:
			break;
		case RC_INPUT: // Include file
		        if (lexrc.next()) {
				string tmp = LibFileSearch(string(),
							   lexrc.GetString()); 
				if (Read(tmp)) {
					lexrc.printError("Error reading "
							"included file: "+tmp);
				}
			}
			break;
		case RC_BINDFILE:                     // RVDK_PATCH_5
			if (lexrc.next()) 
				ReadBindFile(lexrc.GetString());
			break;

		case RC_BEGINTOOLBAR:
			// this toolbar should be changed to be a completely
			// non gui toolbar. (Lgb)
			toolbar.read(lexrc);
			break;
			
		case RC_KBMAP:
			if (lexrc.next())
				use_kbmap = lexrc.GetBool();
			break;

		case RC_EXIT_CONFIRMATION:
			if (lexrc.next())
				exit_confirmation = lexrc.GetBool();
			break;

		case RC_DISPLAY_SHORTCUTS:
			if (lexrc.next())
				display_shortcuts = lexrc.GetBool();
			break;

		case RC_KBMAP_PRIMARY:
			if (lexrc.next())
				primary_kbmap = lexrc.GetString();
			break;

		case RC_KBMAP_SECONDARY:
			if (lexrc.next())
				secondary_kbmap = lexrc.GetString();
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
			
		case RC_LATEX_COMMAND:
			if (lexrc.next())
				latex_command = lexrc.GetString();
			break;

                case RC_LITERATE_COMMAND:
                        if (lexrc.next())
                                literate_command = lexrc.GetString();
                        break;
 
                case RC_LITERATE_EXTENSION:
                        if (lexrc.next())
                                literate_extension = lexrc.GetString();
                        break;

                case RC_LITERATE_ERROR_FILTER:
                        if (lexrc.next())
                                literate_error_filter = lexrc.GetString();
                        break;

                case RC_BUILD_COMMAND:
                        if (lexrc.next())
                                build_command = lexrc.GetString();
                        break;

                case RC_BUILD_ERROR_FILTER:
                        if (lexrc.next())
                                build_error_filter = lexrc.GetString();
                         break;
 
		case RC_RELYX_COMMAND:
			if (lexrc.next())
				relyx_command = lexrc.GetString();
			break;

		case RC_DEFAULT_PAPERSIZE:
                        if (lexrc.next()) {
			        string size = lowercase(lexrc.GetString());
				if (size == "usletter")
				        default_papersize = PAPER_USLETTER;
				else if (size == "legal")
				        default_papersize = PAPER_LEGALPAPER;
				else if (size == "executive")
				        default_papersize = PAPER_EXECUTIVEPAPER;
				else if (size == "a3")
				        default_papersize = PAPER_A3PAPER;
				else if (size == "a4")
				        default_papersize = PAPER_A4PAPER;
				else if (size == "a5")
				        default_papersize = PAPER_A5PAPER;
				else if (size == "b5")
				        default_papersize = PAPER_B5PAPER;
			}
			break;
		case RC_VIEWDVI_COMMAND:
			if (lexrc.next())
				view_dvi_command = lexrc.GetString();
			break;

		case RC_PS_COMMAND:
			if (lexrc.next())
				ps_command = lexrc.GetString();
			break;

		case RC_VIEWPS_COMMAND:
			if (lexrc.next())
				view_ps_command = lexrc.GetString();
			break;

		case RC_VIEWPSPIC_COMMAND:
			if (lexrc.next())
				view_pspic_command = lexrc.GetString();
			break;

		case RC_CHKTEX_COMMAND:
			if (lexrc.next())
				chktex_command = lexrc.GetString();
			break;

		case RC_HTML_COMMAND:
			if (lexrc.next())
				html_command = lexrc.GetString();
			break;

		case RC_SCREEN_DPI:
			if (lexrc.next())
				dpi = lexrc.GetInteger();
			break;

		case RC_SCREEN_ZOOM:
			if (lexrc.next())
				zoom = lexrc.GetInteger();
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
			
		case RC_SGML_EXTRA_OPTIONS:
			if (lexrc.next())
				sgml_extra_options = lexrc.GetString();
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
			if (lexrc.next())
				font_norm = lexrc.GetString();
			break;
			
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
			if (!hasBindFile)
				ReadBindFile();

			// !!!chb, dynamic key binding...
			int action, res=0;
			string seq, cmd;

			if (lexrc.lex()==LyXLex::LEX_DATA)  {
				seq = lexrc.GetString();
			} else {
				lexrc.printError("Bad key sequence: `$$Token'");
				break;
			}

			if (lexrc.lex()==LyXLex::LEX_DATA) {
				cmd = lexrc.GetString();
			} else {
				lexrc.printError("Bad command: `$$Token'");
				break;
			}

			if ((action = lyxaction.LookupFunc(cmd.c_str()))>=0) {
				if (lyxerr.debugging(Debug::KEY)) {
					lyxerr << "RC_BIND: Sequence `"
					       << seq << "' Command `"
					       << cmd << "' Action `"
					       << action << '\'' << endl;
				}
				res = bindKey(seq.c_str(), action);
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
		case RC_SERVERPIPE:
			if (lexrc.next())
				lyxpipes = ExpandPath(lexrc.GetString());
			break;
			
		case RC_CURSOR_FOLLOWS_SCROLLBAR:
			if (lexrc.next())
				cursor_follows_scrollbar = lexrc.GetBool();
			break;
		case RC_FAST_SELECTION:
			if (lexrc.next())
				fast_selection = lexrc.GetBool();
			break;
		case RC_BACKGROUND_COLOR:
			if (lexrc.next())
				background_color = lexrc.GetString();
			break;
		case RC_SELECTION_COLOR:
			if (lexrc.next())
				strncpy(selection_color,
					lexrc.GetString().c_str(),31);
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

		default:
			lexrc.printError("Unknown tag `$$Token'");
			break;
		}
	}

	return 0;
}


void LyXRC::Print()
{
	lyxerr << "The current internal LyXRC:" << endl;
}


/// define the default key bindings for LyX.
void LyXRC::defaultKeyBindings()
{
	bindKey("Right",   LFUN_RIGHT);
	bindKey("Left",    LFUN_LEFT);
	bindKey("Up",      LFUN_UP);
	bindKey("Down",    LFUN_DOWN);

	bindKey("Tab",  LFUN_TAB);

	bindKey("Home",    LFUN_HOME);
	bindKey("End",     LFUN_END);
	bindKey("Prior",   LFUN_PRIOR);
	bindKey("Next",    LFUN_NEXT);

	bindKey("Return",  LFUN_BREAKPARAGRAPH);
	bindKey("~C-~S-~M-nobreakspace", LFUN_PROTECTEDSPACE);

	bindKey("Delete",  LFUN_DELETE);
	bindKey("BackSpace",    LFUN_BACKSPACE);
	// bindKeyings for transparent handling of deadkeys
	// The keysyms are gotten from XFree86 X11R6
	bindKey("~C-~S-~M-dead_acute",           LFUN_ACUTE);
	bindKey("~C-~S-~M-dead_breve",           LFUN_BREVE);
	bindKey("~C-~S-~M-dead_caron",           LFUN_CARON);
	bindKey("~C-~S-~M-dead_cedilla",         LFUN_CEDILLA);
	bindKey("~C-~S-~M-dead_abovering",          LFUN_CIRCLE);
	bindKey("~C-~S-~M-dead_circumflex",      LFUN_CIRCUMFLEX);
	bindKey("~C-~S-~M-dead_abovedot",             LFUN_DOT);
	bindKey("~C-~S-~M-dead_grave",           LFUN_GRAVE);
	bindKey("~C-~S-~M-dead_doubleacute",     LFUN_HUNG_UMLAUT);
	bindKey("~C-~S-~M-dead_macron",          LFUN_MACRON);
	// nothing with this name
	// bindKey("~C-~S-~M-dead_special_caron",   LFUN_SPECIAL_CARON);
	bindKey("~C-~S-~M-dead_tilde",           LFUN_TILDE);
	bindKey("~C-~S-~M-dead_diaeresis",       LFUN_UMLAUT);
	// nothing with this name either...
	//bindKey("~C-~S-~M-dead_underbar",        LFUN_UNDERBAR);
	bindKey("~C-~S-~M-dead_belowdot",        LFUN_UNDERDOT);
	bindKey("~C-~S-~M-dead_tie",             LFUN_TIE);
	bindKey("~C-~S-~M-dead_ogonek",           LFUN_OGONEK);

	// bindings to utilize the use of the numeric keypad
	// e.g. Num Lock set
	bindKey("KP_0",        LFUN_SELFINSERT);
	bindKey("KP_Decimal",  LFUN_SELFINSERT);
	bindKey("KP_Enter",    LFUN_SELFINSERT);
	bindKey("KP_1",        LFUN_SELFINSERT);
	bindKey("KP_2",        LFUN_SELFINSERT);
	bindKey("KP_3",        LFUN_SELFINSERT);
	bindKey("KP_4",        LFUN_SELFINSERT);
	bindKey("KP_5",        LFUN_SELFINSERT);
	bindKey("KP_6",        LFUN_SELFINSERT);
	bindKey("KP_Add",      LFUN_SELFINSERT);
	bindKey("KP_7",        LFUN_SELFINSERT);
	bindKey("KP_8",        LFUN_SELFINSERT);
	bindKey("KP_9",        LFUN_SELFINSERT);
	bindKey("KP_Divide",   LFUN_SELFINSERT);
	bindKey("KP_Multiply", LFUN_SELFINSERT);
	bindKey("KP_Subtract", LFUN_SELFINSERT);
    
	/* Most self-insert keys are handled in the 'default:' section of
	 * WorkAreaKeyPress - so we don't have to define them all.
	 * However keys explicit decleared as self-insert are
	 * handled seperatly (LFUN_SELFINSERT.) Lgb. */

        bindKey("C-Tab",  LFUN_TABINSERT);  // ale970515
}
