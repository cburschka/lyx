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

using std::ostream;
using std::ofstream;
using std::cout;
using std::ios;
using std::endl;

// this is crappy... why are those colors command line arguments and
// not in lyxrc?? (Matthias) 
// Because nobody put them there. (Asger)

extern bool cursor_follows_scrollbar;
extern LyXAction lyxaction;
extern kb_keymap * toplevel_keymap;


enum LyXRCTags {
	RC_BEGINTOOLBAR = 1,
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
	RC_SCREEN_FONT_ENCODING_MENU,
	RC_AUTOSAVE,
	RC_DOCUMENTPATH,
	RC_TEMPLATEPATH,
	RC_TEMPDIRPATH,
	RC_USETEMPDIR,
	RC_LASTFILES,
	RC_AUTOREGIONDELETE,
	RC_BIND,
	RC_OVERRIDE_X_DEADKEYS,
	RC_SERVERPIPE,
	RC_INPUT,
	RC_BINDFILE,
	RC_KBMAP,
	RC_KBMAP_PRIMARY,
	RC_KBMAP_SECONDARY,
	RC_FAX_COMMAND,
	RC_PHONEBOOK,
	RC_FAXPROGRAM,
	RC_ASCIIROFF_COMMAND,
	RC_ASCII_LINELEN,
	RC_NUMLASTFILES,
	RC_CHECKLASTFILES,
	RC_VIEWDVI_COMMAND,
	RC_VIEWDVI_PAPEROPTION,
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
	RC_MAKE_BACKUP,
	RC_BACKUPDIR_PATH,
	RC_RTL_SUPPORT,
	RC_AUTO_NUMBER,
	RC_MARK_FOREIGN_LANGUAGE,
	RC_LANGUAGE_PACKAGE,
	RC_LANGUAGE_AUTO_BEGIN,
	RC_LANGUAGE_AUTO_END,
	RC_LANGUAGE_COMMAND_BEGIN,
	RC_LANGUAGE_COMMAND_END,
	RC_PDFLATEX_COMMAND,
	RC_PDF_MODE,
	RC_VIEWPDF_COMMAND,
	RC_PDF_TO_PS_COMMAND,
	RC_DVI_TO_PS_COMMAND,
	RC_DATE_INSERT_FORMAT,
	RC_SHOW_BANNER,
	RC_USE_GUI,
	RC_LINUXDOC_TO_LYX_COMMAND,
	RC_LINUXDOC_TO_HTML_COMMAND,
	RC_LINUXDOC_TO_LATEX_COMMAND,
	RC_DOCBOOK_TO_DVI_COMMAND,
	RC_DOCBOOK_TO_HTML_COMMAND,
	RC_DOCBOOK_TO_PDF_COMMAND,
	RC_LAST
};


static
keyword_item lyxrcTags[] = {
	{ "\\accept_compound", RC_ACCEPT_COMPOUND },
	{ "\\alternate_language", RC_ALT_LANG },
	{ "\\ascii_linelen", RC_ASCII_LINELEN },
	{ "\\ascii_roff_command", RC_ASCIIROFF_COMMAND },
	{ "\\auto_number", RC_AUTO_NUMBER },
	{ "\\auto_region_delete", RC_AUTOREGIONDELETE },
	{ "\\autosave", RC_AUTOSAVE },
	{ "\\backupdir_path", RC_BACKUPDIR_PATH },
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
	{ "\\date_insert_format", RC_DATE_INSERT_FORMAT },
	{ "\\default_papersize", RC_DEFAULT_PAPERSIZE },
	{ "\\display_shortcuts", RC_DISPLAY_SHORTCUTS },
	{ "\\docbook_to_dvi_command", RC_DOCBOOK_TO_DVI_COMMAND },
	{ "\\docbook_to_html_command", RC_DOCBOOK_TO_HTML_COMMAND },
	{ "\\docbook_to_pdf_command", RC_DOCBOOK_TO_PDF_COMMAND },
	{ "\\document_path", RC_DOCUMENTPATH },
	{ "\\dvi_to_ps_command", RC_DVI_TO_PS_COMMAND },
	{ "\\escape_chars", RC_ESC_CHARS },
	{ "\\exit_confirmation", RC_EXIT_CONFIRMATION },
	{ "\\fax_command", RC_FAX_COMMAND },
	{ "\\fax_program", RC_FAXPROGRAM },
	{ "\\font_encoding", RC_FONT_ENCODING },
	{ "\\html_command", RC_HTML_COMMAND },
	{ "\\input", RC_INPUT },
	{ "\\kbmap", RC_KBMAP },
	{ "\\kbmap_primary", RC_KBMAP_PRIMARY },
	{ "\\kbmap_secondary", RC_KBMAP_SECONDARY },
	{ "\\language_auto_begin", RC_LANGUAGE_AUTO_BEGIN },
	{ "\\language_auto_end", RC_LANGUAGE_AUTO_END },
	{ "\\language_command_begin", RC_LANGUAGE_COMMAND_BEGIN },
	{ "\\language_command_end", RC_LANGUAGE_COMMAND_END },
	{ "\\language_package", RC_LANGUAGE_PACKAGE },
	{ "\\lastfiles", RC_LASTFILES },
	{ "\\latex_command", RC_LATEX_COMMAND },
	{ "\\linuxdoc_to_html_command", RC_LINUXDOC_TO_HTML_COMMAND },
	{ "\\linuxdoc_to_latex_command", RC_LINUXDOC_TO_LATEX_COMMAND },
	{ "\\linuxdoc_to_lyx_command", RC_LINUXDOC_TO_LYX_COMMAND },
        { "\\literate_command", RC_LITERATE_COMMAND },
        { "\\literate_error_filter", RC_LITERATE_ERROR_FILTER },
        { "\\literate_extension", RC_LITERATE_EXTENSION },
	{ "\\make_backup", RC_MAKE_BACKUP },
	{ "\\mark_foreign_language", RC_MARK_FOREIGN_LANGUAGE },
	{ "\\num_lastfiles", RC_NUMLASTFILES },
	{ "\\override_x_deadkeys", RC_OVERRIDE_X_DEADKEYS },
	{ "\\pdf_mode", RC_PDF_MODE },
	{ "\\pdf_to_ps_command", RC_PDF_TO_PS_COMMAND },
	{ "\\pdflatex_command", RC_PDFLATEX_COMMAND },
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
	{ "\\rtl", RC_RTL_SUPPORT },
	{ "\\screen_dpi", RC_SCREEN_DPI },
	{ "\\screen_font_encoding", RC_SCREEN_FONT_ENCODING },
	{ "\\screen_font_encoding_menu", RC_SCREEN_FONT_ENCODING_MENU },
	{ "\\screen_font_menu", RC_SCREEN_FONT_MENU },
	{ "\\screen_font_popup", RC_SCREEN_FONT_POPUP },
	{ "\\screen_font_roman", RC_SCREEN_FONT_ROMAN },
	{ "\\screen_font_sans", RC_SCREEN_FONT_SANS },
	{ "\\screen_font_scalable", RC_SCREEN_FONT_SCALABLE },
	{ "\\screen_font_sizes", RC_SCREEN_FONT_SIZES },
	{ "\\screen_font_typewriter", RC_SCREEN_FONT_TYPEWRITER },
	{ "\\screen_zoom", RC_SCREEN_ZOOM },
	{ "\\serverpipe", RC_SERVERPIPE },
	{ "\\show_banner", RC_SHOW_BANNER },
	{ "\\spell_command", RC_SPELL_COMMAND },
	{ "\\tempdir_path", RC_TEMPDIRPATH },
	{ "\\template_path", RC_TEMPLATEPATH },
	{ "\\use_alt_language", RC_USE_ALT_LANG },
	{ "\\use_escape_chars", RC_USE_ESC_CHARS },
	{ "\\use_gui", RC_USE_GUI },
	{ "\\use_input_encoding", RC_USE_INP_ENC },
	{ "\\use_personal_dictionary", RC_USE_PERS_DICT },
	{ "\\use_tempdir", RC_USETEMPDIR },
	{ "\\view_dvi_command", RC_VIEWDVI_COMMAND },
	{ "\\view_dvi_paper_option", RC_VIEWDVI_PAPEROPTION },
	{ "\\view_pdf_command", RC_VIEWPDF_COMMAND },
	{ "\\view_ps_command", RC_VIEWPS_COMMAND },
        { "\\view_pspic_command", RC_VIEWPSPIC_COMMAND }
};

/* Let the range depend of the size of lyxrcTags.  Alejandro 240596 */
static const int lyxrcCount = sizeof(lyxrcTags) / sizeof(keyword_item);


LyXRC::LyXRC() 
{
	setDefaults();
}


void LyXRC::setDefaults() {
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
	pdf_mode = false;
	latex_command = "latex";
	pdflatex_command = "pdflatex";
	pdf_to_ps_command = "pdf2ps";
	dvi_to_ps_command = "dvips";
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
	view_dvi_paper_option = "-paper";
	view_pdf_command = "xpdf";
	default_papersize = BufferParams::PAPER_USLETTER;
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
	font_norm_menu = "";
	override_x_deadkeys = true;
	autosave = 300;
	auto_region_delete = true;
	ascii_linelen = 75;
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

	///
	date_insert_format = "%A, %e %B %Y";
	show_banner = true;
	use_gui = true;

	///
	linuxdoc_to_latex_command="none";
	linuxdoc_to_lyx_command="none";
	linuxdoc_to_html_command="none";

	docbook_to_dvi_command="none";
	docbook_to_html_command="none";
	docbook_to_pdf_command="none";
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
				ReadBindFile(lexrc.GetString());
			}
			break;
			
		case RC_BEGINTOOLBAR:
			// this toolbar should be changed to be a completely
			// non gui toolbar. (Lgb)
			toolbardefaults.read(lexrc);
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

		case RC_PDF_MODE:
			if (lexrc.next())
				pdf_mode = lexrc.GetBool();
			break;
			
		case RC_LATEX_COMMAND:
			if (lexrc.next())
				latex_command = lexrc.GetString();
			break;

		case RC_PDFLATEX_COMMAND:
			if (lexrc.next())
				pdflatex_command = lexrc.GetString();
			break;

		case RC_PDF_TO_PS_COMMAND:
			if (lexrc.next())
				pdf_to_ps_command = lexrc.GetString();
			break;

		case RC_DVI_TO_PS_COMMAND:
			if (lexrc.next())
				dvi_to_ps_command = lexrc.GetString();
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
		case RC_VIEWDVI_COMMAND:
			if (lexrc.next())
				view_dvi_command = lexrc.GetString();
			break;

		case RC_VIEWDVI_PAPEROPTION:
			if (lexrc.next())
				view_dvi_paper_option = lexrc.GetString();
			else 
				view_dvi_paper_option = "";
			break;

		case RC_VIEWPDF_COMMAND:
			if (lexrc.next())
				view_pdf_command = lexrc.GetString();
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

		case RC_SCREEN_FONT_ENCODING_MENU:
			if (lexrc.next())
				font_norm_menu = lexrc.GetString();
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
			
			if ((action = lyxaction.LookupFunc(cmd.c_str()))>= 0) {
				if (lyxerr.debugging(Debug::KEY)) {
					lyxerr << "RC_BIND: Sequence `"
					       << seq << "' Command `"
					       << cmd << "' Action `"
					       << action << '\'' << endl;
				}
				res = toplevel_keymap->bind(seq.c_str(),
							    action);
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
		case RC_USE_GUI:
			if (lexrc.next())
				use_gui = lexrc.GetBool();
			break;
		case RC_LINUXDOC_TO_LYX_COMMAND:
			if ( lexrc.next())
				linuxdoc_to_lyx_command = lexrc.GetString();
			break;
		case RC_LINUXDOC_TO_HTML_COMMAND:
			if ( lexrc.next())
				linuxdoc_to_html_command = lexrc.GetString();
			break;
		case RC_LINUXDOC_TO_LATEX_COMMAND:
			if ( lexrc.next())
				linuxdoc_to_latex_command = lexrc.GetString();
			break;
		case RC_DOCBOOK_TO_DVI_COMMAND:
			if ( lexrc.next())
				docbook_to_dvi_command = lexrc.GetString();
			break;
		case RC_DOCBOOK_TO_HTML_COMMAND:
			if ( lexrc.next())
				docbook_to_html_command = lexrc.GetString();
			break;
		case RC_DOCBOOK_TO_PDF_COMMAND:
			if ( lexrc.next())
				docbook_to_pdf_command = lexrc.GetString();
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
	   << "###          Copyright 1995-1998 The LyX Team.\n"
	   << "###\n"
	   << "### ========================================================\n"
	   << "\n"
	   << "# This file is written by LyX, if you want to make your own\n"
	   << "# modifications you should do them from inside LyX and save\n"
	   << "# your preferences, or you can also make your modifications\n"
	   << "# to lyxrc by hand. It is not advisable to edit this file.\n"
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
		// bind files are not done here.
	case RC_BEGINTOOLBAR:
		// Toolbar is not written here (yet).
	case RC_FONT_ENCODING:
		os << "\\font_encoding \"" << fontenc << "\"\n";
	case RC_PRINTER:
		os << "\\printer \"" << printer << "\"\n";
	case RC_PRINT_COMMAND:
		os << "\\print_command \"" << print_command << "\"\n";
	case RC_PRINTEVENPAGEFLAG:
		os << "\\print_evenpage_flag \"" << print_evenpage_flag
		   << "\"\n";
	case RC_PRINTODDPAGEFLAG:
		os << "\\print_oddpage_flag \"" << print_oddpage_flag
		   << "\"\n";
	case RC_PRINTPAGERANGEFLAG:
		os << "\\print_pagerange_flag \"" << print_pagerange_flag
		   << "\"\n";
	case RC_PRINTCOPIESFLAG:
		os << "\\print_copies_flag \"" << print_copies_flag << "\"\n";
	case RC_PRINTCOLLCOPIESFLAG:
		os << "\\print_collcopies_flag \"" << print_collcopies_flag
		   << "\"\n";
	case RC_PRINTREVERSEFLAG:
		os << "\\print_reverse_flag \"" << print_reverse_flag
		   << "\"\n";
	case RC_PRINTLANDSCAPEFLAG:
		os << "\\print_landscape_flag \"" << print_landscape_flag
		   << "\"\n";
	case RC_PRINTTOPRINTER:
		os << "\\print_to_printer \"" << print_to_printer << "\"\n";
	case RC_PRINT_ADAPTOUTPUT:
		os << "\\print_adapt_output " << tostr(print_adapt_output)
		   << "\n";
	case RC_PRINTTOFILE:
		os << "\\print_to_file \"" << print_to_file << "\"\n";
	case RC_PRINTFILEEXTENSION:
		os << "\\print_file_extension \"" << print_file_extension
		   << "\"\n";
	case RC_PRINTEXSTRAOPTIONS:
		os << "\\print_extra_options \"" << print_extra_options
		   << "\"\n";
	case RC_PRINTSPOOL_COMMAND:
		os << "\\print_spool_command \"" << print_spool_command
		   << "\"\n";
	case RC_PRINTSPOOL_PRINTERPREFIX:
		os << "\\print_spool_printerprefix \""
		   << print_spool_printerprefix << "\"\n";
	case RC_PRINTPAPERDIMENSIONFLAG:
		os << "\\print_paper_dimension_flag \""
		   << print_paper_dimension_flag << "\"\n";
	case RC_PRINTPAPERFLAG:
		os << "\\print_paper_flag \"" << print_paper_flag << "\"\n";
	case RC_CUSTOM_EXPORT_COMMAND:
		os << "\\custom_export_command \"" << custom_export_command
		   << "\"\n";
	case RC_CUSTOM_EXPORT_FORMAT:
		os << "\\custom_export_format \"" << custom_export_format
		   << "\"\n";
	case RC_PDF_MODE:
		os << "\\pdf_mode " << tostr(pdf_mode) << "\n";
	case RC_LATEX_COMMAND:
		os << "\\latex_command \"" << latex_command << "\"\n";
	case RC_PDFLATEX_COMMAND:
		os << "\\pdflatex_command \"" << pdflatex_command << "\"\n";
	case RC_PDF_TO_PS_COMMAND:
		os << "\\pdf_to_ps_command \"" << pdf_to_ps_command << "\"\n";
	case RC_DVI_TO_PS_COMMAND:
		os << "\\dvi_to_ps_command \"" << dvi_to_ps_command << "\"\n";
	case RC_LITERATE_COMMAND:
		os << "\\literate_command \"" << literate_command << "\"\n";
	case RC_LITERATE_EXTENSION:
		os << "\\literate_extension \"" << literate_extension
		   << "\"\n";
	case RC_LITERATE_ERROR_FILTER:
		os << "\\literate_error_filter \"" << literate_error_filter
		   << "\"\n";
	case RC_BUILD_COMMAND:
		os << "\\build_command \"" << build_command << "\"\n";
	case RC_BUILD_ERROR_FILTER:
		os << "\\build_error_filter \"" << build_error_filter
		   << "\"\n";
	case RC_SCREEN_DPI:
		os << "\\screen_dpi " << dpi << "\n";
	case RC_SCREEN_ZOOM:
		os << "\\screen_zoom " << zoom << "\n";
	case RC_SCREEN_FONT_SIZES:
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
	case RC_OVERRIDE_X_DEADKEYS:
		os << "\\override_x_deadkeys " 
		   << override_x_deadkeys << "\n";
	case RC_AUTOREGIONDELETE:
		os << "\\auto_region_delete " << tostr(auto_region_delete)
		   << "\n";
	case RC_AUTOSAVE:
		os << "\\autosave " << autosave << "\n";
	case RC_EXIT_CONFIRMATION:
		os << "\\exit_confirmation " << tostr(exit_confirmation)
		   << "\n";
	case RC_DISPLAY_SHORTCUTS:
		os << "\\display_shortcuts " << tostr(display_shortcuts)
		   << "\n";
	case RC_VIEWDVI_COMMAND:
		os << "\\view_dvi_command \"" << view_dvi_command << "\"\n";
	case RC_VIEWDVI_PAPEROPTION:
		os << "\\view_dvi_paper_option \"" << view_dvi_paper_option << "\"\n";
	case RC_VIEWPDF_COMMAND:
		os << "\\view_pdf_command \"" << view_pdf_command << "\"\n";
	case RC_DEFAULT_PAPERSIZE:
		os << "\\default_papersize \"";
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
	case RC_VIEWPS_COMMAND:
		os << "\\view_ps_command \"" << view_ps_command << "\"\n";
	case RC_VIEWPSPIC_COMMAND:
		os << "\\view_pspic_command \"" << view_pspic_command
		   << "\"\n";
	case RC_PS_COMMAND:
		os << "\\ps_command \"" << ps_command << "\"\n";
	case RC_CHKTEX_COMMAND:
		os << "\\chktex_command \"" << chktex_command << "\"\n";
	case RC_HTML_COMMAND:
		os << "\\html_command \"" << html_command << "\"\n";
	case RC_KBMAP:
		os << "\\kbmap " << tostr(use_kbmap) << "\n";
	case RC_KBMAP_PRIMARY:
		os << "\\kbmap_primary \"" << primary_kbmap << "\"\n";
	case RC_KBMAP_SECONDARY:
		os << "\\kbmap_secondary \"" << secondary_kbmap << "\"\n";
	case RC_SERVERPIPE:
		os << "\\serverpipe \"" << lyxpipes << "\"\n";
	case RC_RELYX_COMMAND:
		os << "\\relyx_command \"" << relyx_command << "\"\n";
	case RC_DOCUMENTPATH:
		os << "\\document_path \"" << document_path << "\"\n";
	case RC_TEMPLATEPATH:
		os << "\\template_path \"" << template_path << "\"\n";
	case RC_TEMPDIRPATH:
		os << "\\tempdir_path \"" << tempdir_path << "\"\n";
	case RC_USETEMPDIR:
		os << "\\use_tempdir " << tostr(use_tempdir) << "\n";
	case RC_LASTFILES:
		os << "\\lastfiles \"" << lastfiles << "\"\n";
	case RC_NUMLASTFILES:
		os << "\\num_lastfiles " << num_lastfiles << "\n";
	case RC_CHECKLASTFILES:
		os << "\\check_lastfiles " << tostr(check_lastfiles) << "\n";
	case RC_SCREEN_FONT_ROMAN:
		os << "\\screen_font_roman \"" << roman_font_name << "\"\n";
	case RC_SCREEN_FONT_SANS:
		os << "\\screen_font_sans \"" << sans_font_name << "\"\n";
	case RC_SCREEN_FONT_TYPEWRITER:
		os << "\\screen_font_typewriter \""
		   << typewriter_font_name << "\"\n";
	case RC_SCREEN_FONT_MENU:
		os << "\\screen_font_menu \"" << menu_font_name << "\"\n";
	case RC_SCREEN_FONT_POPUP:
		os << "\\screen_font_popup \"" << popup_font_name << "\"\n";
	case RC_SCREEN_FONT_ENCODING:
		os << "\\screen_font_encoding \"" << font_norm << "\"\n";
	case RC_SCREEN_FONT_ENCODING_MENU:
		os << "\\screen_font_encoding_menu \"" << font_norm_menu
		   << "\"\n";
	case RC_SCREEN_FONT_SCALABLE:
		os << "\\screen_font_scalable " << tostr(use_scalable_fonts)
		   << "\n";
	case RC_CURSOR_FOLLOWS_SCROLLBAR:
		os << "\\cursor_follows_scrollbar "
		   << tostr(cursor_follows_scrollbar) << "\n";
	case RC_FAX_COMMAND:
		os << "\\fax_command \"" << fax_command << "\"\n";
	case RC_FAXPROGRAM:
		os << "\\fax_program \"" << fax_program << "\"\n";
	case RC_PHONEBOOK:
		os << "\\phone_book \"" << phone_book << "\"\n";
	case RC_ASCIIROFF_COMMAND:
		os << "\\ascii_roff_command \"" << ascii_roff_command
		   << "\"\n";
	case RC_ASCII_LINELEN:
		os << "\\ascii_linelen " << ascii_linelen << "\n";
	case RC_SPELL_COMMAND:
		os << "\\spell_command \"" << isp_command << "\"\n";
	case RC_ACCEPT_COMPOUND:
		os << "\\accept_compound " << tostr(isp_accept_compound)
		   << "\n";
	case RC_USE_INP_ENC:
		os << "\\use_input_encoding " << tostr(isp_use_input_encoding)
		   << "\n";
	case RC_USE_ALT_LANG:
		os << "\\use_alt_language " << tostr(isp_use_alt_lang) << "\n";
	case RC_USE_PERS_DICT:
		os << "\\use_personal_dictionary " << tostr(isp_use_pers_dict)
		   << "\n";
	case RC_USE_ESC_CHARS:
		os << "\\use_escape_chars " << tostr(isp_use_esc_chars)
		   << "\n";
	case RC_ALT_LANG:
		os << "\\alternate_language \"" << isp_alt_lang << "\"\n";
	case RC_PERS_DICT:
		os << "\\personal_dictionary \"" << isp_pers_dict << "\"\n";
	case RC_ESC_CHARS:
		os << "\\escape_chars \"" << isp_esc_chars << "\"\n";
	case RC_RTL_SUPPORT:
		os << "\\rtl " << tostr(rtl_support) << "\n";
	case RC_AUTO_NUMBER:
		os << "\\auto_number" << tostr(auto_number) << "\n";
	case RC_MARK_FOREIGN_LANGUAGE:
		os << "\\mark_foreign_language " << 
			tostr(mark_foreign_language) << "\n";
	case RC_LANGUAGE_AUTO_BEGIN:
		os << "\\language_auto_begin " 
		   << tostr(language_auto_begin) << "\n";
	case RC_LANGUAGE_AUTO_END:
		os << "\\language_auto_end " 
		   << tostr(language_auto_end) << "\n";
	case RC_LANGUAGE_PACKAGE:
		os << "\\language_package \"" << language_package << "\"\n";
	case RC_LANGUAGE_COMMAND_BEGIN:
		os << "\\language_command_begin \"" << language_command_begin
		   << "\"\n";
	case RC_LANGUAGE_COMMAND_END:
		os << "\\language_command_end \"" << language_command_end
		   << "\"\n";
	case RC_MAKE_BACKUP:
		os << "\\make_backup " << tostr(make_backup) << "\n";
	case RC_BACKUPDIR_PATH:
		os << "\\backupdir_path" << backupdir_path << "\n";
	case RC_DATE_INSERT_FORMAT:
		os << "\\date_insert_format \"" << date_insert_format
		   << "\"\n";
	case RC_SHOW_BANNER:
		os << "\\show_banner " << tostr(show_banner) << "\n";
	case RC_USE_GUI:
		os << "\\use_gui " << tostr(show_banner) << "\n";
	case RC_LINUXDOC_TO_LYX_COMMAND:
		os << "\\linuxdoc_to_lyx_command \"" << linuxdoc_to_lyx_command
		   << "\"\n";
	case RC_LINUXDOC_TO_HTML_COMMAND:
		os << "\\linuxdoc_to_html_command \"" << linuxdoc_to_html_command
		   << "\"\n";
	case RC_LINUXDOC_TO_LATEX_COMMAND:
		os << "\\linuxdoc_to_latex_command \"" << linuxdoc_to_latex_command
		   << "\"\n";
	case RC_DOCBOOK_TO_DVI_COMMAND:
		os << "\\docbook_to_dvi_command \"" << docbook_to_dvi_command
		   << "\"\n";
	case RC_DOCBOOK_TO_HTML_COMMAND:
		os << "\\docbook_to_html_command \"" << docbook_to_html_command
		   << "\"\n";
	case RC_DOCBOOK_TO_PDF_COMMAND:
		os << "\\docbook_to_pdf_command \"" << docbook_to_pdf_command
		   << "\"\n";
	}
	os.flush();
}


// The global instance
LyXRC lyxrc;
