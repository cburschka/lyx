/**
 * \file LyXRC.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Allan Rae
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <fstream>
#include <iostream>

#include "LyXRC.h"

#include "Color.h"
#include "Converter.h"
#include "Format.h"
#include "Session.h"
#include "Lexer.h"
#include "FontEnums.h"
#include "Mover.h"

#include "graphics/GraphicsTypes.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/environment.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/userinfo.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace os = support::os;

namespace {

// when adding something to this array keep it sorted!
LexerKeyword lyxrcTags[] = {
	{ "\\accept_compound", LyXRC::RC_ACCEPT_COMPOUND },
	{ "\\allow_geometry_session", LyXRC::RC_GEOMETRY_SESSION },
	{ "\\alternate_language", LyXRC::RC_ALT_LANG },
	{ "\\auto_number", LyXRC::RC_AUTO_NUMBER },
	{ "\\auto_region_delete", LyXRC::RC_AUTOREGIONDELETE },
	{ "\\auto_reset_options", LyXRC::RC_AUTORESET_OPTIONS },
	{ "\\autosave", LyXRC::RC_AUTOSAVE },
	{ "\\backupdir_path", LyXRC::RC_BACKUPDIR_PATH },
	{ "\\bibtex_command", LyXRC::RC_BIBTEX_COMMAND },
	{ "\\bind_file", LyXRC::RC_BINDFILE },
	{ "\\check_lastfiles", LyXRC::RC_CHECKLASTFILES },
	{ "\\chktex_command", LyXRC::RC_CHKTEX_COMMAND },
	{ "\\completion_cursor_text", LyXRC::RC_COMPLETION_CURSOR_TEXT },
	{ "\\completion_inline_delay", LyXRC::RC_COMPLETION_INLINE_DELAY },
	{ "\\completion_inline_dots", LyXRC::RC_COMPLETION_INLINE_DOTS },
	{ "\\completion_inline_math", LyXRC::RC_COMPLETION_INLINE_MATH },
	{ "\\completion_inline_text", LyXRC::RC_COMPLETION_INLINE_TEXT },
	{ "\\completion_popup_after_complete", LyXRC::RC_COMPLETION_POPUP_AFTER_COMPLETE },
	{ "\\completion_popup_delay", LyXRC::RC_COMPLETION_POPUP_DELAY },
	{ "\\completion_popup_math", LyXRC::RC_COMPLETION_POPUP_MATH },
	{ "\\completion_popup_text", LyXRC::RC_COMPLETION_POPUP_TEXT },
	{ "\\converter", LyXRC::RC_CONVERTER },
	{ "\\converter_cache_maxage", LyXRC::RC_CONVERTER_CACHE_MAXAGE },
	{ "\\copier", LyXRC::RC_COPIER },
	{ "\\cursor_follows_scrollbar", LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR },
	{ "\\custom_export_command", LyXRC::RC_CUSTOM_EXPORT_COMMAND },
	{ "\\custom_export_format", LyXRC::RC_CUSTOM_EXPORT_FORMAT },
	{ "\\date_insert_format", LyXRC::RC_DATE_INSERT_FORMAT },
	{ "\\def_file", LyXRC::RC_DEFFILE },
	{ "\\default_language", LyXRC::RC_DEFAULT_LANGUAGE },
	{ "\\default_papersize", LyXRC::RC_DEFAULT_PAPERSIZE },
	{ "\\dialogs_iconify_with_main", LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN },
	{ "\\display_graphics", LyXRC::RC_DISPLAY_GRAPHICS },
	{ "\\document_path", LyXRC::RC_DOCUMENTPATH },
	{ "\\escape_chars", LyXRC::RC_ESC_CHARS },
	{ "\\example_path", LyXRC::RC_EXAMPLEPATH },
	{ "\\font_encoding", LyXRC::RC_FONT_ENCODING },
	{ "\\format", LyXRC::RC_FORMAT },
	{ "\\fullscreen_limit", LyXRC::RC_FULL_SCREEN_LIMIT },
	{ "\\fullscreen_scrollbar", LyXRC::RC_FULL_SCREEN_SCROLLBAR },
	{ "\\fullscreen_tabbar", LyXRC::RC_FULL_SCREEN_TABBAR },
	{ "\\fullscreen_toolbars", LyXRC::RC_FULL_SCREEN_TOOLBARS },
	{ "\\fullscreen_width", LyXRC::RC_FULL_SCREEN_WIDTH },
	{ "\\group_layouts", LyXRC::RC_GROUP_LAYOUTS },
	{ "\\index_command", LyXRC::RC_INDEX_COMMAND },
	{ "\\input", LyXRC::RC_INPUT },
	{ "\\kbmap", LyXRC::RC_KBMAP },
	{ "\\kbmap_primary", LyXRC::RC_KBMAP_PRIMARY },
	{ "\\kbmap_secondary", LyXRC::RC_KBMAP_SECONDARY },
	{ "\\label_init_length", LyXRC::RC_LABEL_INIT_LENGTH },
	{ "\\language_auto_begin", LyXRC::RC_LANGUAGE_AUTO_BEGIN },
	{ "\\language_auto_end", LyXRC::RC_LANGUAGE_AUTO_END },
	{ "\\language_command_begin", LyXRC::RC_LANGUAGE_COMMAND_BEGIN },
	{ "\\language_command_end", LyXRC::RC_LANGUAGE_COMMAND_END },
	{ "\\language_command_local", LyXRC::RC_LANGUAGE_COMMAND_LOCAL },
	{ "\\language_global_options", LyXRC::RC_LANGUAGE_GLOBAL_OPTIONS },
	{ "\\language_package", LyXRC::RC_LANGUAGE_PACKAGE },
	{ "\\language_use_babel", LyXRC::RC_LANGUAGE_USE_BABEL },
	{ "\\load_session", LyXRC::RC_LOADSESSION },
	{ "\\macro_edit_style", LyXRC::RC_MACRO_EDIT_STYLE },
	{ "\\make_backup", LyXRC::RC_MAKE_BACKUP },
	{ "\\mark_foreign_language", LyXRC::RC_MARK_FOREIGN_LANGUAGE },
	{ "\\mouse_wheel_speed", LyXRC::RC_MOUSE_WHEEL_SPEED },
	{ "\\num_lastfiles", LyXRC::RC_NUMLASTFILES },
	{ "\\open_buffers_in_tabs", LyXRC::RC_OPEN_BUFFERS_IN_TABS },
	{ "\\path_prefix", LyXRC::RC_PATH_PREFIX },
	{ "\\personal_dictionary", LyXRC::RC_PERS_DICT },
	{ "\\plaintext_linelen", LyXRC::RC_PLAINTEXT_LINELEN },
	{ "\\plaintext_roff_command", LyXRC::RC_PLAINTEXT_ROFF_COMMAND },
	{ "\\preview", LyXRC::RC_PREVIEW },
	{ "\\preview_hashed_labels", LyXRC::RC_PREVIEW_HASHED_LABELS },
	{ "\\preview_scale_factor", LyXRC::RC_PREVIEW_SCALE_FACTOR },
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
	{ "\\rtl", LyXRC::RC_RTL_SUPPORT },
	{ "\\screen_dpi", LyXRC::RC_SCREEN_DPI },
	{ "\\screen_font_roman", LyXRC::RC_SCREEN_FONT_ROMAN },
	{ "\\screen_font_roman_foundry", LyXRC::RC_SCREEN_FONT_ROMAN_FOUNDRY },
	{ "\\screen_font_sans", LyXRC::RC_SCREEN_FONT_SANS },
	{ "\\screen_font_sans_foundry", LyXRC::RC_SCREEN_FONT_SANS_FOUNDRY },
	{ "\\screen_font_scalable", LyXRC::RC_SCREEN_FONT_SCALABLE },
	{ "\\screen_font_sizes", LyXRC::RC_SCREEN_FONT_SIZES },
	{ "\\screen_font_typewriter", LyXRC::RC_SCREEN_FONT_TYPEWRITER },
	{ "\\screen_font_typewriter_foundry", LyXRC::RC_SCREEN_FONT_TYPEWRITER_FOUNDRY },
	{ "\\screen_zoom", LyXRC::RC_SCREEN_ZOOM },
	{ "\\serverpipe", LyXRC::RC_SERVERPIPE },
	{ "\\set_color", LyXRC::RC_SET_COLOR },
	{ "\\show_banner", LyXRC::RC_SHOW_BANNER },
	{ "\\sort_layouts", LyXRC::RC_SORT_LAYOUTS },
	{ "\\spell_command", LyXRC::RC_SPELL_COMMAND },
	{ "\\tempdir_path", LyXRC::RC_TEMPDIRPATH },
	{ "\\template_path", LyXRC::RC_TEMPLATEPATH },
	{ "\\tex_allows_spaces", LyXRC::RC_TEX_ALLOWS_SPACES },
	{ "\\tex_expects_windows_paths", LyXRC::RC_TEX_EXPECTS_WINDOWS_PATHS },
	{ "\\ui_file", LyXRC::RC_UIFILE },
	{ "\\use_alt_language", LyXRC::RC_USE_ALT_LANG },
	{ "\\use_converter_cache", LyXRC::RC_USE_CONVERTER_CACHE },
	{ "\\use_escape_chars", LyXRC::RC_USE_ESC_CHARS },
	{ "\\use_input_encoding", LyXRC::RC_USE_INP_ENC },
	{ "\\use_lastfilepos", LyXRC::RC_USELASTFILEPOS },
	{ "\\use_personal_dictionary", LyXRC::RC_USE_PERS_DICT },
	{ "\\use_pixmap_cache", LyXRC::RC_USE_PIXMAP_CACHE },
	// compatibility with versions older than 1.4.0 only
	{ "\\use_pspell", LyXRC::RC_USE_SPELL_LIB },
	{ "\\use_spell_lib", LyXRC::RC_USE_SPELL_LIB },
	// compatibility with versions older than 1.4.0 only
	{ "\\use_tempdir", LyXRC::RC_USETEMPDIR },
	{ "\\use_tooltip", LyXRC::RC_USE_TOOLTIP },
	{ "\\user_email", LyXRC::RC_USER_EMAIL },
	{ "\\user_name", LyXRC::RC_USER_NAME },
	{ "\\view_dvi_paper_option", LyXRC::RC_VIEWDVI_PAPEROPTION },
	// compatibility with versions older than 1.4.0 only
	{ "\\viewer", LyXRC::RC_VIEWER},
	{ "\\visual_cursor" ,LyXRC::RC_VISUAL_CURSOR}
};

const int lyxrcCount = sizeof(lyxrcTags) / sizeof(lyxrcTags[0]);

} // namespace anon


LyXRC::LyXRC()
{
	setDefaults();
}


void LyXRC::setDefaults()
{
	bind_file = "cua";
	def_file = "default";
	ui_file = "default";
	// Get printer from the environment. If fail, use default "",
	// assuming that everything is set up correctly.
	printer = getEnv("PRINTER");
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
	document_path.erase();
	view_dvi_paper_option.erase();
	default_papersize = PAPER_DEFAULT;
	custom_export_format = "ps";
	chktex_command = "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38";
	bibtex_command = "bibtex";
	fontenc = "default";
	index_command = "makeindex -c -q";
	dpi = 75;
	// Because a screen typically is wider than a piece of paper:
	zoom = 150;
	allow_geometry_session = true;
	// Default LaTeX font size:
	font_sizes[FONT_SIZE_TINY] = "5.0";
	font_sizes[FONT_SIZE_SCRIPT] = "7.0";
	font_sizes[FONT_SIZE_FOOTNOTE] = "8.0";
	font_sizes[FONT_SIZE_SMALL] = "9.0";
	font_sizes[FONT_SIZE_NORMAL] = "10.0";
	font_sizes[FONT_SIZE_LARGE] = "12.0";
	font_sizes[FONT_SIZE_LARGER] = "14.4";
	font_sizes[FONT_SIZE_LARGEST] = "17.26";
	font_sizes[FONT_SIZE_HUGE] = "20.74";
	font_sizes[FONT_SIZE_HUGER] = "24.88";
	use_scalable_fonts = true;
	roman_font_name = "";
	sans_font_name = "";
	typewriter_font_name = "";
	autosave = 300;
	auto_region_delete = true;
	auto_reset_options = false;
	plaintext_linelen = 65;
	mouse_wheel_speed = 1.0;
	num_lastfiles = maxlastfiles;
	check_lastfiles = true;
	use_lastfilepos = true;
	load_session = false;
	make_backup = true;
	backupdir_path.erase();
	display_graphics = true;
	// Spellchecker settings:
	use_spell_lib = true;
	isp_command = "ispell";
	isp_accept_compound = false;
	isp_use_input_encoding = false;
	isp_use_alt_lang = false;
	isp_use_pers_dict = false;
	isp_use_esc_chars = false;
	use_kbmap = false;
	rtl_support = true;
	visual_cursor = false;
	auto_number = true;
	mark_foreign_language = true;
	language_auto_begin = true;
	language_auto_end = true;
	language_global_options = true;
	language_use_babel = true;
	language_package = "\\usepackage{babel}";
	language_command_begin = "\\selectlanguage{$$lang}";
	language_command_local = "\\foreignlanguage{$$lang}{";
	sort_layouts = false;
	group_layouts = true;
	default_language = "english";
	show_banner = true;
	windows_style_tex_paths = false;
	tex_allows_spaces = false;
	date_insert_format = "%x";
	cursor_follows_scrollbar = false;
	macro_edit_style = MACRO_EDIT_INLINE_BOX;
	dialogs_iconify_with_main = false;
	label_init_length = 3;
	preview = PREVIEW_OFF;
	preview_hashed_labels  = false;
	preview_scale_factor = "0.9";
	use_converter_cache = true;
	use_tooltip = true;
	use_pixmap_cache = false;
	converter_cache_maxage = 6 * 30 * 24 * 3600; // 6 months
	user_name = to_utf8(support::user_name());
	user_email = to_utf8(support::user_email());
	open_buffers_in_tabs = true;

	// Fullscreen settings
	full_screen_limit = false;
	full_screen_toolbars = true;
	full_screen_tabbar = true;
	full_screen_scrollbar = true;
	full_screen_width = 700;

	completion_cursor_text = true;
	completion_popup_math = true;
	completion_popup_text = false;
	completion_popup_delay = 2.0;
	completion_popup_after_complete = true;
	completion_inline_math = true;
	completion_inline_text = false;
	completion_inline_dots = -1;
	completion_inline_delay = 0.2;
}


namespace {

void oldFontFormat(string & family, string & foundry)
{
	if (family.empty() || family[0] != '-')
		return;
	foundry = token(family, '-', 1);
	family = token(family, '-', 2);
	if (foundry == "*")
		foundry.erase();
}

} // namespace anon


int LyXRC::read(FileName const & filename)
{
	Lexer lexrc(lyxrcTags);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);

	lexrc.setFile(filename);
	if (!lexrc.isOK())
		return -2;

	LYXERR(Debug::LYXRC, "Reading '" << filename << "'...");

	return read(lexrc);
}


int LyXRC::read(istream & is)
{
	Lexer lexrc(lyxrcTags);
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);

	lexrc.setStream(is);
	if (!lexrc.isOK())
		return -2;

	LYXERR(Debug::LYXRC, "Reading istream...");

	return read(lexrc);
}


int LyXRC::read(Lexer & lexrc)
{
	if (!lexrc.isOK()) return -2;

	while (lexrc.isOK()) {
		// By using two switches we take advantage of the compiler
		// telling us if we have missed a LyXRCTags element in
		// the second switch.
		// Note that this also shows a problem with Lexer since it
		// helps us avoid taking advantage of the strictness of the
		// compiler.

		int le = lexrc.lex();
		switch (le) {
		case Lexer::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			continue;
		case Lexer::LEX_FEOF:
			continue;
		default: break;
		}
		switch (static_cast<LyXRCTags>(le)) {
		case RC_INPUT: // Include file
			if (lexrc.next()) {
				FileName const tmp =
					libFileSearch(string(),
						      lexrc.getString());
				if (read(tmp)) {
					lexrc.printError("Error reading "
							 "included file: " + tmp.absFilename());
				}
			}
			break;
		case RC_BINDFILE:
			if (lexrc.next()) {
				bind_file = os::internal_path(lexrc.getString());
			}
			break;

		case RC_DEFFILE:
			if (lexrc.next()) {
				def_file = os::internal_path(lexrc.getString());
			}
			break;

		case RC_UIFILE:
			if (lexrc.next()) {
				ui_file = os::internal_path(lexrc.getString());
			}
			break;

		case RC_AUTORESET_OPTIONS:
			lexrc >> auto_reset_options;
			break;

		case RC_DISPLAY_GRAPHICS:
			if (lexrc.next())
				display_graphics = lexrc.getString() == "true";
			break;

		case RC_TEX_EXPECTS_WINDOWS_PATHS:
			lexrc >> windows_style_tex_paths;
			break;

		case RC_TEX_ALLOWS_SPACES:
			lexrc >> tex_allows_spaces;
			break;

		case RC_KBMAP:
			lexrc >> use_kbmap;
			break;

		case RC_KBMAP_PRIMARY:
			if (lexrc.next()) {
				string const kmap(os::internal_path(lexrc.getString()));
				if (kmap.empty()) {
					// nothing
				} else if (!libFileSearch("kbd", kmap,
							  "kmap").empty()) {
					primary_kbmap = kmap;
				} else {
					lexrc.printError("LyX: Keymap `$$Token' not found");
				}
			}
			break;

		case RC_KBMAP_SECONDARY:
			if (lexrc.next()) {
				string const kmap(os::internal_path(lexrc.getString()));
				if (kmap.empty()) {
					// nothing
				} else if (!libFileSearch("kbd", kmap,
							  "kmap").empty()) {
					secondary_kbmap = kmap;
				} else {
					lexrc.printError("LyX: Keymap `$$Token' not found");
				}
			}
			break;

		case RC_FONT_ENCODING:
			lexrc >> fontenc;
			break;

		case RC_PRINTER:
			lexrc >> printer;
			break;

		case RC_PRINT_COMMAND:
			if (lexrc.next(true)) {
				print_command = lexrc.getString();
			}
			break;

		case RC_PRINTEVENPAGEFLAG:
			lexrc >> print_evenpage_flag;
			break;

		case RC_PRINTODDPAGEFLAG:
			lexrc >> print_oddpage_flag;
			break;

		case RC_PRINTPAGERANGEFLAG:
			lexrc >> print_pagerange_flag;
			break;

		case RC_PRINTCOPIESFLAG:
			lexrc >> print_copies_flag;
			break;

		case RC_PRINTCOLLCOPIESFLAG:
			lexrc >> print_collcopies_flag;
			break;

		case RC_PRINTREVERSEFLAG:
			lexrc >> print_reverse_flag;
			break;

		case RC_PRINTLANDSCAPEFLAG:
			lexrc >> print_landscape_flag;
			break;

		case RC_PRINTTOPRINTER:
			lexrc >> print_to_printer;
			break;

		case RC_PRINT_ADAPTOUTPUT:
			lexrc >> print_adapt_output;
			break;

		case RC_PRINTTOFILE:
			if (lexrc.next()) {
				print_to_file = os::internal_path(lexrc.getString());
			}
			break;

		case RC_PRINTFILEEXTENSION:
			lexrc >> print_file_extension;
			break;

		case RC_PRINTEXSTRAOPTIONS:
			lexrc >> print_extra_options;
			break;

		case RC_PRINTSPOOL_COMMAND:
			if (lexrc.next(true)) {
				print_spool_command = lexrc.getString();
			}
			break;

		case RC_PRINTSPOOL_PRINTERPREFIX:
			lexrc >> print_spool_printerprefix;
			break;

		case RC_PRINTPAPERDIMENSIONFLAG:
			lexrc >> print_paper_dimension_flag;
			break;

		case RC_PRINTPAPERFLAG:
			lexrc >> print_paper_flag;
			break;

		case RC_CUSTOM_EXPORT_COMMAND:
			lexrc >> custom_export_command;
			break;

		case RC_CUSTOM_EXPORT_FORMAT:
			lexrc >> custom_export_format;
			break;

		case RC_DEFAULT_PAPERSIZE:
			if (lexrc.next()) {
				string const size = ascii_lowercase(lexrc.getString());
				if (size == "usletter")
					default_papersize = PAPER_USLETTER;
				else if (size == "legal")
					default_papersize = PAPER_USLEGAL;
				else if (size == "executive")
					default_papersize = PAPER_USEXECUTIVE;
				else if (size == "a3")
					default_papersize = PAPER_A3;
				else if (size == "a4")
					default_papersize = PAPER_A4;
				else if (size == "a5")
					default_papersize = PAPER_A5;
				else if (size == "b5")
					default_papersize = PAPER_B5;
				else if (size == "default")
					default_papersize = PAPER_DEFAULT;
			}
			break;

		case RC_VIEWDVI_PAPEROPTION:
			if (lexrc.next())
				view_dvi_paper_option = lexrc.getString();
			else
				view_dvi_paper_option.erase();
			break;

		case RC_CHKTEX_COMMAND:
			if (lexrc.next(true)) {
				chktex_command = lexrc.getString();
			}
			break;

		case RC_BIBTEX_COMMAND:
			if (lexrc.next(true)) {
				bibtex_command = lexrc.getString();
			}
			break;

		case RC_INDEX_COMMAND:
			if (lexrc.next(true)) {
				index_command = lexrc.getString();
			}
			break;

		case RC_SCREEN_DPI:
			lexrc >> dpi;
			break;

		case RC_SCREEN_ZOOM:
			lexrc >> zoom;
			break;

		case RC_GEOMETRY_SESSION:
			lexrc >> allow_geometry_session;
			break;

		case RC_SCREEN_FONT_SIZES:
			lexrc >> font_sizes[FONT_SIZE_TINY];
			lexrc >> font_sizes[FONT_SIZE_SCRIPT];
			lexrc >> font_sizes[FONT_SIZE_FOOTNOTE];
			lexrc >> font_sizes[FONT_SIZE_SMALL];
			lexrc >> font_sizes[FONT_SIZE_NORMAL];
			lexrc >> font_sizes[FONT_SIZE_LARGE];
			lexrc >> font_sizes[FONT_SIZE_LARGER];
			lexrc >> font_sizes[FONT_SIZE_LARGEST];
			lexrc >> font_sizes[FONT_SIZE_HUGE];
			lexrc >> font_sizes[FONT_SIZE_HUGER];
			break;

		case RC_SCREEN_FONT_SCALABLE:
			lexrc >> use_scalable_fonts;
			break;

		case RC_AUTOSAVE:
			lexrc >> autosave;
			break;

		case RC_DOCUMENTPATH:
			if (lexrc.next()) {
				document_path = os::internal_path(lexrc.getString());
				document_path = expandPath(document_path);
			}
			break;

		case RC_EXAMPLEPATH:
			if (lexrc.next()) {
				example_path = os::internal_path(lexrc.getString());
				example_path = expandPath(example_path);
			}
			break;

		case RC_TEMPLATEPATH:
			if (lexrc.next()) {
				template_path = os::internal_path(lexrc.getString());
				template_path = expandPath(template_path);
			}
			break;

		case RC_TEMPDIRPATH:
			if (lexrc.next()) {
				tempdir_path = os::internal_path(lexrc.getString());
				tempdir_path = expandPath(tempdir_path);
			}
			break;

		case RC_USETEMPDIR:
			if (lexrc.next())
				LYXERR0("Ignoring obsolete use_tempdir flag.");
			break;

		case RC_USELASTFILEPOS:
			lexrc >> use_lastfilepos;
			break;

		case RC_LOADSESSION:
			lexrc >> load_session;
			break;

		case RC_MOUSE_WHEEL_SPEED:
			lexrc >> mouse_wheel_speed;
			break;

		case RC_COMPLETION_INLINE_DELAY:
			lexrc >> completion_inline_delay;
			break;

		case RC_COMPLETION_INLINE_MATH:
			lexrc >> completion_inline_math;
			break;

		case RC_COMPLETION_INLINE_TEXT:
			lexrc >> completion_inline_text;
			break;

		case RC_COMPLETION_INLINE_DOTS:
			lexrc >> completion_inline_dots;
			break;

		case RC_COMPLETION_POPUP_DELAY:
			lexrc >> completion_popup_delay;
			break;

		case RC_COMPLETION_POPUP_MATH:
			lexrc >> completion_popup_math;
			break;

		case RC_COMPLETION_POPUP_TEXT:
			lexrc >> completion_popup_text;
			break;

		case RC_COMPLETION_CURSOR_TEXT:
			lexrc >> completion_cursor_text;
			break;

		case RC_COMPLETION_POPUP_AFTER_COMPLETE:
			lexrc >> completion_popup_after_complete;
			break;

		case RC_NUMLASTFILES:
			lexrc >> num_lastfiles;
			break;

		case RC_CHECKLASTFILES:
			lexrc >> check_lastfiles;
			break;

		case RC_SCREEN_FONT_ROMAN:
			if (lexrc.next()) {
				roman_font_name = lexrc.getString();
				oldFontFormat(roman_font_name,
					      roman_font_foundry);
			}
			break;

		case RC_SCREEN_FONT_SANS:
			if (lexrc.next()) {
				sans_font_name = lexrc.getString();
				oldFontFormat(sans_font_name, sans_font_foundry);
			}
			break;

		case RC_SCREEN_FONT_TYPEWRITER:
			if (lexrc.next()) {
				typewriter_font_name = lexrc.getString();
				oldFontFormat(typewriter_font_name,
					      typewriter_font_foundry);
			}
			break;

		case RC_SCREEN_FONT_ROMAN_FOUNDRY:
			lexrc >> roman_font_foundry;
			break;

		case RC_SCREEN_FONT_SANS_FOUNDRY:
			lexrc >> sans_font_foundry;
			break;

		case RC_SCREEN_FONT_TYPEWRITER_FOUNDRY:
			lexrc >> typewriter_font_foundry;
			break;

		case RC_SET_COLOR: {
			if (!lexrc.next()) {
				lexrc.printError("Missing color tag.");
				break;
			}
			string lyx_name = lexrc.getString();

			if (!lexrc.next()) {
				lexrc.printError("Missing color name for color: `$$Token'");
				break;
			}
			string x11_name = lexrc.getString();

			ColorCode const col =
				lcolor.getFromLyXName(lyx_name);
			if (col == Color_none ||
			    col == Color_inherit ||
			    col == Color_ignore)
				break;

			if (!lcolor.setColor(col, x11_name))
				LYXERR0("Bad lyxrc set_color for " << lyx_name);
			break;
		}

		case RC_AUTOREGIONDELETE:
			// Auto region delete defaults to true
			lexrc >> auto_region_delete;
			break;

		case RC_SERVERPIPE:
			if (lexrc.next()) {
				lyxpipes = os::internal_path(lexrc.getString());
				lyxpipes = expandPath(lyxpipes);
			}
			break;

		case RC_CURSOR_FOLLOWS_SCROLLBAR:
			lexrc >> cursor_follows_scrollbar;
			break;

		case RC_MACRO_EDIT_STYLE:
			if (lexrc.next()) {
				switch (lexrc.getInteger()) {
				case 0: macro_edit_style = MACRO_EDIT_INLINE_BOX; break;
				case 1: macro_edit_style = MACRO_EDIT_INLINE; break;
				case 2: macro_edit_style = MACRO_EDIT_LIST; break;
				}
			}
			break;

		case RC_DIALOGS_ICONIFY_WITH_MAIN:
			lexrc >> dialogs_iconify_with_main;
			break;

		case RC_PLAINTEXT_ROFF_COMMAND:
			if (lexrc.next(true)) {
				plaintext_roff_command = lexrc.getString();
			}
			break;
		case RC_PLAINTEXT_LINELEN:
			lexrc >> plaintext_linelen;
			break;
			// Spellchecker settings:
		case RC_USE_SPELL_LIB:
			lexrc >> use_spell_lib;
			break;
		case RC_SPELL_COMMAND:
			if (lexrc.next(true))
				isp_command = lexrc.getString();
			break;
		case RC_ACCEPT_COMPOUND:
			lexrc >> isp_accept_compound;
			break;
		case RC_USE_INP_ENC:
			lexrc >> isp_use_input_encoding;
			break;
		case RC_USE_ALT_LANG:
			lexrc >> isp_use_alt_lang;
			break;
		case RC_USE_PERS_DICT:
			lexrc >> isp_use_pers_dict;
			break;
		case RC_USE_TOOLTIP:
			lexrc >> use_tooltip;
			break;
		case RC_USE_PIXMAP_CACHE:
			lexrc >> use_pixmap_cache;
			break;
		case RC_USE_ESC_CHARS:
			lexrc >> isp_use_esc_chars;
			break;
		case RC_ALT_LANG:
			lexrc >> isp_alt_lang;
			break;
		case RC_PERS_DICT:
			if (lexrc.next())
				isp_pers_dict = os::internal_path(lexrc.getString());
			break;
		case RC_ESC_CHARS:
			lexrc >> isp_esc_chars;
			break;
		case RC_MAKE_BACKUP:
			lexrc >> make_backup;
			break;
		case RC_BACKUPDIR_PATH:
			if (lexrc.next()) {
				backupdir_path = os::internal_path(lexrc.getString());
				backupdir_path = expandPath(backupdir_path);
			}
			break;
		case RC_DATE_INSERT_FORMAT:
			lexrc >> date_insert_format;
			break;
		case RC_LANGUAGE_PACKAGE:
			lexrc >> language_package;
			break;
		case RC_LANGUAGE_AUTO_BEGIN:
			lexrc >> language_auto_begin;
			break;
		case RC_LANGUAGE_AUTO_END:
			lexrc >> language_auto_end;
			break;
		case RC_LANGUAGE_GLOBAL_OPTIONS:
			lexrc >> language_global_options;
			break;
		case RC_LANGUAGE_USE_BABEL:
			lexrc >> language_use_babel;
			break;
		case RC_LANGUAGE_COMMAND_BEGIN:
			lexrc >> language_command_begin;
			break;
		case RC_LANGUAGE_COMMAND_END:
			lexrc >> language_command_end;
			break;
		case RC_LANGUAGE_COMMAND_LOCAL:
			lexrc >> language_command_local;
			break;
		case RC_RTL_SUPPORT:
			lexrc >> rtl_support;
			break;
		case RC_VISUAL_CURSOR:
			lexrc >> visual_cursor;
			break;
		case RC_AUTO_NUMBER:
			lexrc >> auto_number;
			break;
		case RC_MARK_FOREIGN_LANGUAGE:
			lexrc >> mark_foreign_language;
			break;

		case RC_COPIER: {
			string fmt, command;
			if (lexrc.next())
				fmt = lexrc.getString();
			if (lexrc.next(true))
				command = lexrc.getString();
			setMover(fmt, command);
			break;
		}

		case RC_CONVERTER: {
			string from, to, command, flags;
			if (lexrc.next())
				from = lexrc.getString();
			if (lexrc.next())
				to = lexrc.getString();
			if (lexrc.next(true))
				command = lexrc.getString();
			if (lexrc.next())
				flags = lexrc.getString();
			if (command.empty())
				theConverters().erase(from, to);
			else
				theConverters().add(from, to, command, flags);
			break;
		}
		// compatibility with versions older than 1.4.0 only
		case RC_VIEWER: {
			string format, command;
			lexrc >> format >> command;
			formats.setViewer(format, command);
			break;
		}
		case RC_FORMAT: {
			string format, extension, prettyname, shortcut;
			lexrc >> format >> extension >> prettyname >> shortcut;
			string viewer, editor;
			if (lexrc.next(true))
				viewer = lexrc.getString();
			if (lexrc.next(true))
				editor = lexrc.getString();
			string flags;
			// Hack to ensure compatibility with versions older
			// than 1.5.0
			int le = lexrc.lex();
			if (le != Lexer::LEX_FEOF && le != Lexer::LEX_UNDEF) {
				flags = lexrc.getString();
				if (le != Lexer::LEX_DATA) {
					// We have got a known token.
					// Therefore this is an old style
					// format definition without
					// flags.
					lexrc.pushToken(flags);
					flags.erase();
				}
			}
			int flgs = Format::none;
			while (!flags.empty()) {
				string flag;
				flags = split(flags, flag, ',');
				if (flag == "document")
					flgs |= Format::document;
				else if (flag == "vector")
					flgs |= Format::vector;
				else
					LYXERR0("Ignoring unknown flag `"
					       << flag << "' for format `"
					       << format << "'.");
			}
			if (prettyname.empty()) {
				if (theConverters().formatIsUsed(format))
					LYXERR0("Can't delete format " << format);
				else
					formats.erase(format);
			} else {
				formats.add(format, extension, prettyname,
					    shortcut, viewer, editor, flgs);
			}
			break;
		}
		case RC_DEFAULT_LANGUAGE:
			lexrc >> default_language;
			break;

		case RC_LABEL_INIT_LENGTH:
			lexrc >> label_init_length;
			break;

		case RC_SHOW_BANNER:
			lexrc >> show_banner;
			break;

		case RC_PREVIEW:
			if (lexrc.next()) {
				string const tmp = lexrc.getString();
				if (tmp == "true" || tmp == "on")
					preview = PREVIEW_ON;
				else if (tmp == "no_math")
					preview = PREVIEW_NO_MATH;
				else {
					preview = PREVIEW_OFF;
					if (tmp != "false" && tmp != "off")
						LYXERR0("Unrecognized preview status \""
						       << tmp << '\n');
				}
			}
			break;

		case RC_PREVIEW_HASHED_LABELS:
			lexrc >> preview_hashed_labels;
			break;

		case RC_PREVIEW_SCALE_FACTOR:
			lexrc >> preview_scale_factor;
			break;

		case RC_USER_NAME:
			lexrc >> user_name;
			break;
		case RC_USER_EMAIL:
			lexrc >> user_email;
			break;

		case RC_PATH_PREFIX:
			lexrc >> path_prefix;
			break;

		case RC_USE_CONVERTER_CACHE:
			lexrc >> use_converter_cache;
			break;
		case RC_CONVERTER_CACHE_MAXAGE:
			lexrc >> converter_cache_maxage;
			break;

		case RC_SORT_LAYOUTS:
			lexrc >> sort_layouts;
			break;
		case RC_GROUP_LAYOUTS:
			lexrc >> group_layouts;
			break;
		case RC_FULL_SCREEN_LIMIT:
			lexrc >> full_screen_limit;
			break;
		case RC_FULL_SCREEN_TOOLBARS:
			lexrc >> full_screen_toolbars;
			break;
		case RC_FULL_SCREEN_SCROLLBAR:
			lexrc >> full_screen_scrollbar;
			break;
		case RC_FULL_SCREEN_TABBAR:
			lexrc >> full_screen_tabbar;
			break;
		case RC_FULL_SCREEN_WIDTH:
			lexrc >> full_screen_width;
			break;
		case RC_OPEN_BUFFERS_IN_TABS:
			lexrc >> open_buffers_in_tabs;
			break;

		case RC_LAST:
			break; // this is just a dummy
		}
	}

	/// Update converters data-structures
	theConverters().update(formats);
	theConverters().buildGraph();

	return 0;
}


void LyXRC::write(FileName const & filename, bool ignore_system_lyxrc) const
{
	ofstream ofs(filename.toFilesystemEncoding().c_str());
	if (ofs)
		write(ofs, ignore_system_lyxrc);
}


void LyXRC::print() const
{
	if (lyxerr.debugging())
		write(lyxerr, false);
	else
		write(cout, false);
}


class SameMover {
public:
	typedef pair<string, SpecialisedMover> Data;

	SameMover(Data const & comparison)
		: comparison_(comparison) {}

	bool operator()(Data const & data) const
	{
		return data.first == comparison_.first &&
			data.second.command() == comparison_.second.command();
	}

private:
	Data comparison_;
};


namespace {

	// Escape \ and " so that LyXLex can read the string later
	string escapeCommand(string const & str) {
		return subst(subst(str , "\\", "\\\\"), 
			     "\"", "\\\"");
	}

}


void LyXRC::write(ostream & os, bool ignore_system_lyxrc, string const & name) const
{
	LyXRCTags tag = RC_LAST;
	
	if (!name.empty()) {
		for (int i = 0; i != lyxrcCount; ++i)
			if ("\\" + name == lyxrcTags[i].tag)
				tag = static_cast<LyXRCTags>(lyxrcTags[i].code);
	}

	if (tag == RC_LAST)
		os << "# This file is written by LyX, if you want to make your own\n"
		   << "# modifications you should do them from inside LyX and save\n"
		   << "\n";

	// Why the switch you might ask. It is a trick to ensure that all
	// the elements in the LyXRCTags enum are handled. As you can see
	// there are no breaks at all. So it is just a huge fall-through.
	// The nice thing is that we will get a warning from the compiler
	// if we forget an element.
	switch (tag) {
	case RC_LAST:
	case RC_INPUT:
		// input/include files are not done here
	case RC_BINDFILE:
		if (ignore_system_lyxrc ||
		    bind_file != system_lyxrc.bind_file) {
			string const path = os::external_path(bind_file);
			os << "\\bind_file \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;

	case RC_DEFFILE:
		if (ignore_system_lyxrc ||
		    def_file != system_lyxrc.def_file) {
			string const path = os::external_path(def_file);
			os << "\\def_file \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;

		//
		// Misc Section
		//
		os << "\n#\n"
		   << "# MISC SECTION ######################################\n"
		   << "#\n\n";
		// bind files are not done here.

	case RC_PATH_PREFIX:
		if (ignore_system_lyxrc ||
		    path_prefix != system_lyxrc.path_prefix) {
			os << "\\path_prefix \"" << path_prefix << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_UIFILE:
		if (ignore_system_lyxrc ||
		    ui_file != system_lyxrc.ui_file) {
			string const path = os::external_path(ui_file);
			os << "\\ui_file \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_AUTOREGIONDELETE:
		if (ignore_system_lyxrc ||
		    auto_region_delete != system_lyxrc.auto_region_delete) {
			os << "# Set to false to inhibit automatic replacement of\n"
			   << "# the current selection.\n"
			   << "\\auto_region_delete " << convert<string>(auto_region_delete)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_AUTORESET_OPTIONS:
		if (ignore_system_lyxrc ||
		    auto_reset_options != system_lyxrc.auto_reset_options) {
			os << "# Set to false to inhibit automatic reset of\n"
			   << "# the class options to defaults on class change.\n"
			   << "\\auto_reset_options "
			   << convert<string>(auto_reset_options)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_AUTOSAVE:
		if (ignore_system_lyxrc ||
		    autosave != system_lyxrc.autosave) {
			os << "# The time interval between auto-saves in seconds.\n"
			   << "\\autosave " << autosave << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_DISPLAY_GRAPHICS:
		if (ignore_system_lyxrc ||
		    display_graphics != system_lyxrc.display_graphics) {
			os << "# Display graphics within LyX\n"
			   << "# true|false\n"
			   << "\\display_graphics "
			   << (display_graphics ? "true" : "false")
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SORT_LAYOUTS:
		if (ignore_system_lyxrc ||
		    sort_layouts != system_lyxrc.sort_layouts) {
			os << "# Sort layouts alphabetically.\n"
			   << "\\sort_layouts " << convert<string>(sort_layouts) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_GROUP_LAYOUTS:
		if (ignore_system_lyxrc ||
		    group_layouts != system_lyxrc.group_layouts) {
			os << "# Group layouts by their category.\n"
			   << "\\group_layouts " << convert<string>(group_layouts) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_VIEWDVI_PAPEROPTION:
		if (ignore_system_lyxrc ||
		    view_dvi_paper_option
		    != system_lyxrc.view_dvi_paper_option) {
			os << "# Options used to specify paper size to the\n"
			   << "# view_dvi_command (e.g. -paper)\n"
			   << "\\view_dvi_paper_option \""
			   << view_dvi_paper_option << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_DEFAULT_PAPERSIZE:
		if (ignore_system_lyxrc ||
		    default_papersize != system_lyxrc.default_papersize) {
			os << "# The default papersize to use.\n"
			   << "\\default_papersize \"";
			switch (default_papersize) {
			case PAPER_DEFAULT:
				os << "default"; break;
			case PAPER_USLETTER:
				os << "usletter"; break;
			case PAPER_USLEGAL:
				os << "legal"; break;
			case PAPER_USEXECUTIVE:
				os << "executive"; break;
			case PAPER_A3:
				os << "a3"; break;
			case PAPER_A4:
				os << "a4"; break;
			case PAPER_A5:
				os << "a5"; break;
			case PAPER_B5:
				os << "b5"; break;
			case PAPER_CUSTOM:
			case PAPER_B3:
			case PAPER_B4: break;
			}
			os << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_CHKTEX_COMMAND:
		if (ignore_system_lyxrc ||
		    chktex_command != system_lyxrc.chktex_command) {
			os << "\\chktex_command \"" << escapeCommand(chktex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_BIBTEX_COMMAND:
		if (ignore_system_lyxrc ||
		    bibtex_command != system_lyxrc.bibtex_command) {
			os << "\\bibtex_command \"" << escapeCommand(bibtex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_INDEX_COMMAND:
		if (ignore_system_lyxrc ||
		    index_command != system_lyxrc.index_command) {
			os << "\\index_command \"" << escapeCommand(index_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_TEX_EXPECTS_WINDOWS_PATHS:
		if (ignore_system_lyxrc ||
		    windows_style_tex_paths != system_lyxrc.windows_style_tex_paths) {
			os << "\\tex_expects_windows_paths "
			   << convert<string>(windows_style_tex_paths) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_TEX_ALLOWS_SPACES:
		if (tex_allows_spaces != system_lyxrc.tex_allows_spaces) {
			os << "\\tex_allows_spaces "
			   << convert<string>(tex_allows_spaces) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_KBMAP:
		if (ignore_system_lyxrc ||
		    use_kbmap != system_lyxrc.use_kbmap) {
			os << "\\kbmap " << convert<string>(use_kbmap) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_KBMAP_PRIMARY:
		if (ignore_system_lyxrc ||
		    primary_kbmap != system_lyxrc.primary_kbmap) {
			string const path = os::external_path(primary_kbmap);
			os << "\\kbmap_primary \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_KBMAP_SECONDARY:
		if (ignore_system_lyxrc ||
		    secondary_kbmap != system_lyxrc.secondary_kbmap) {
			string const path = os::external_path(secondary_kbmap);
			os << "\\kbmap_secondary \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SERVERPIPE:
		if (ignore_system_lyxrc ||
		    lyxpipes != system_lyxrc.lyxpipes) {
			string const path = os::external_path(lyxpipes);
			os << "\\serverpipe \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_DATE_INSERT_FORMAT:
		if (ignore_system_lyxrc ||
		    date_insert_format != system_lyxrc.date_insert_format) {
			os << "\\date_insert_format \"" << date_insert_format
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LABEL_INIT_LENGTH:
		if (ignore_system_lyxrc ||
		    label_init_length != system_lyxrc.label_init_length) {
			os << "\\label_init_length " << label_init_length
			   << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_USER_NAME:
		os << "\\user_name \"" << user_name << "\"\n";
		if (tag != RC_LAST)
			break;

	case RC_USER_EMAIL:
		os << "\\user_email \"" << user_email << "\"\n";
		if (tag != RC_LAST)
			break;

	case RC_SHOW_BANNER:
		if (ignore_system_lyxrc ||
		    show_banner != system_lyxrc.show_banner) {
			os << "\\show_banner " << convert<string>(show_banner) << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_PREVIEW:
		if (ignore_system_lyxrc ||
		    preview != system_lyxrc.preview) {
			string status;
			switch (preview) {
			case PREVIEW_ON:
				status = "on";
				break;
			case PREVIEW_NO_MATH:
				status = "no_math";
				break;
			case PREVIEW_OFF:
				status = "off";
				break;
			}
			os << "\\preview " << status << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_PREVIEW_HASHED_LABELS:
		if (ignore_system_lyxrc ||
		    preview_hashed_labels !=
		    system_lyxrc.preview_hashed_labels) {
			os << "\\preview_hashed_labels "
			   << convert<string>(preview_hashed_labels) << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_PREVIEW_SCALE_FACTOR:
		if (ignore_system_lyxrc ||
		    preview_scale_factor != system_lyxrc.preview_scale_factor) {
			os << "\\preview_scale_factor "
			   << preview_scale_factor << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_USE_CONVERTER_CACHE:
		if (ignore_system_lyxrc ||
		    use_converter_cache != system_lyxrc.use_converter_cache) {
			os << "\\use_converter_cache "
			   << convert<string>(use_converter_cache) << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_CONVERTER_CACHE_MAXAGE:
		if (ignore_system_lyxrc ||
		    converter_cache_maxage != system_lyxrc.converter_cache_maxage) {
			os << "\\converter_cache_maxage "
			   << converter_cache_maxage << '\n';
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# SCREEN & FONTS SECTION ############################\n"
		   << "#\n\n";

	case RC_SCREEN_DPI:
		if (ignore_system_lyxrc ||
		    dpi != system_lyxrc.dpi) {
			os << "\\screen_dpi " << dpi << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_ZOOM:
		if (ignore_system_lyxrc ||
		    zoom != system_lyxrc.zoom) {
			os << "\\screen_zoom " << zoom << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_GEOMETRY_SESSION:
		if (ignore_system_lyxrc ||
		    allow_geometry_session != system_lyxrc.allow_geometry_session) {
			os << "\\allow_geometry_session " << convert<string>(allow_geometry_session)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_CURSOR_FOLLOWS_SCROLLBAR:
		if (ignore_system_lyxrc ||
		    cursor_follows_scrollbar
		    != system_lyxrc.cursor_follows_scrollbar) {
			os << "\\cursor_follows_scrollbar "
			   << convert<string>(cursor_follows_scrollbar) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_MACRO_EDIT_STYLE:
		if (ignore_system_lyxrc ||
		    macro_edit_style
		    != system_lyxrc.macro_edit_style) {
			os << "\\macro_edit_style ";
			switch (macro_edit_style) {
			case MACRO_EDIT_INLINE_BOX: os << "0\n"; break;
			case MACRO_EDIT_INLINE: os << "1\n"; break;
			case MACRO_EDIT_LIST: os << "2\n"; break;
			}
		}
		if (tag != RC_LAST)
			break;
	case RC_DIALOGS_ICONIFY_WITH_MAIN:
		if (ignore_system_lyxrc ||
		    dialogs_iconify_with_main
		   != system_lyxrc.dialogs_iconify_with_main) {
			os << "\\dialogs_iconify_with_main "
			  <<  convert<string>(dialogs_iconify_with_main) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_ROMAN:
		if (ignore_system_lyxrc ||
		    roman_font_name != system_lyxrc.roman_font_name) {
			os << "\\screen_font_roman \"" << roman_font_name
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_ROMAN_FOUNDRY:
		if (ignore_system_lyxrc ||
		    roman_font_foundry != system_lyxrc.roman_font_foundry) {
			os << "\\screen_font_roman_foundry \"" << roman_font_foundry
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_SANS:
		if (ignore_system_lyxrc ||
		    sans_font_name != system_lyxrc.sans_font_name) {
			os << "\\screen_font_sans \"" << sans_font_name
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_SANS_FOUNDRY:
		if (ignore_system_lyxrc ||
		    sans_font_foundry != system_lyxrc.sans_font_foundry) {
			os << "\\screen_font_sans_foundry \"" << sans_font_foundry
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_TYPEWRITER:
		if (ignore_system_lyxrc ||
		    typewriter_font_name != system_lyxrc.typewriter_font_name) {
			os << "\\screen_font_typewriter \""
			   << typewriter_font_name << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_TYPEWRITER_FOUNDRY:
		if (ignore_system_lyxrc ||
		    typewriter_font_foundry != system_lyxrc.typewriter_font_foundry) {
			os << "\\screen_font_typewriter_foundry \""
			   << typewriter_font_foundry << "\"\n";
		}
		if (tag != RC_LAST)
			break;

	case RC_SCREEN_FONT_SCALABLE:
		if (ignore_system_lyxrc ||
		    use_scalable_fonts != system_lyxrc.use_scalable_fonts) {
			os << "\\screen_font_scalable "
			   << convert<string>(use_scalable_fonts)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SCREEN_FONT_SIZES:
		if (ignore_system_lyxrc ||
		    font_sizes[FONT_SIZE_TINY]
		    != system_lyxrc.font_sizes[FONT_SIZE_TINY] ||
		    font_sizes[FONT_SIZE_SCRIPT]
		    != system_lyxrc.font_sizes[FONT_SIZE_SCRIPT] ||
		    font_sizes[FONT_SIZE_FOOTNOTE]
		    != system_lyxrc.font_sizes[FONT_SIZE_FOOTNOTE] ||
		    font_sizes[FONT_SIZE_SMALL]
		    != system_lyxrc.font_sizes[FONT_SIZE_SMALL] ||
		    font_sizes[FONT_SIZE_NORMAL]
		    != system_lyxrc.font_sizes[FONT_SIZE_NORMAL] ||
		    font_sizes[FONT_SIZE_LARGE]
		    != system_lyxrc.font_sizes[FONT_SIZE_LARGE] ||
		    font_sizes[FONT_SIZE_LARGER]
		    != system_lyxrc.font_sizes[FONT_SIZE_LARGER] ||
		    font_sizes[FONT_SIZE_LARGEST]
		    != system_lyxrc.font_sizes[FONT_SIZE_LARGEST] ||
		    font_sizes[FONT_SIZE_HUGE]
		    != system_lyxrc.font_sizes[FONT_SIZE_HUGE] ||
		    font_sizes[FONT_SIZE_HUGER]
		    != system_lyxrc.font_sizes[FONT_SIZE_HUGER]) {
			os.setf(ios::fixed);
			os.precision(2);
			os << "\\screen_font_sizes"
			   << ' ' << font_sizes[FONT_SIZE_TINY]
			   << ' ' << font_sizes[FONT_SIZE_SCRIPT]
			   << ' ' << font_sizes[FONT_SIZE_FOOTNOTE]
			   << ' ' << font_sizes[FONT_SIZE_SMALL]
			   << ' ' << font_sizes[FONT_SIZE_NORMAL]
			   << ' ' << font_sizes[FONT_SIZE_LARGE]
			   << ' ' << font_sizes[FONT_SIZE_LARGER]
			   << ' ' << font_sizes[FONT_SIZE_LARGEST]
			   << ' ' << font_sizes[FONT_SIZE_HUGE]
			   << ' ' << font_sizes[FONT_SIZE_HUGER]
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_FULL_SCREEN_LIMIT:
		if (ignore_system_lyxrc ||
		    full_screen_limit != system_lyxrc.full_screen_limit) {
			os << "\\fullscreen_limit "
			   << convert<string>(full_screen_limit)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_FULL_SCREEN_TOOLBARS:
		if (ignore_system_lyxrc ||
		    full_screen_toolbars != system_lyxrc.full_screen_toolbars) {
			os << "\\fullscreen_toolbars "
			   << convert<string>(full_screen_toolbars)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_FULL_SCREEN_SCROLLBAR:
		if (ignore_system_lyxrc ||
		    full_screen_scrollbar != system_lyxrc.full_screen_scrollbar) {
			os << "\\fullscreen_scrollbar "
			   << convert<string>(full_screen_scrollbar)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_FULL_SCREEN_TABBAR:
		if (ignore_system_lyxrc ||
		    full_screen_tabbar != system_lyxrc.full_screen_tabbar) {
			os << "\\fullscreen_tabbar "
			   << convert<string>(full_screen_tabbar)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_FULL_SCREEN_WIDTH:
		if (ignore_system_lyxrc ||
		    full_screen_width != system_lyxrc.full_screen_width) {
			os << "\\fullscreen_width "
			   << convert<string>(full_screen_width)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_OPEN_BUFFERS_IN_TABS:
		if (ignore_system_lyxrc ||
		    open_buffers_in_tabs != system_lyxrc.open_buffers_in_tabs) {
			os << "\\open_buffers_in_tabs "
			   << convert<string>(open_buffers_in_tabs)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;

	os << "\n#\n"
			<< "# COLOR SECTION ###################################\n"
			<< "#\n\n";

	case RC_SET_COLOR:
		for (int i = 0; i < Color_ignore; ++i) {
			ColorCode lc = static_cast<ColorCode>(i);
			string const col = lcolor.getX11Name(lc);
			if (ignore_system_lyxrc
			    || col != system_lcolor.getX11Name(lc)) {
				os << "\\set_color \""
				   << lcolor.getLyXName(lc) << "\" \""
				   << col << "\"\n";
			}
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# PRINTER SECTION ###################################\n"
		   << "#\n\n";

	case RC_PRINTER:
		if (ignore_system_lyxrc ||
		    printer != system_lyxrc.printer) {
			os << "\\printer \"" << printer << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINT_ADAPTOUTPUT:
		if (ignore_system_lyxrc ||
		    print_adapt_output != system_lyxrc.print_adapt_output) {
			os << "\\print_adapt_output "
			   << convert<string>(print_adapt_output)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINT_COMMAND:
		if (ignore_system_lyxrc ||
		    print_command != system_lyxrc.print_command) {
			os << "\\print_command \"" << escapeCommand(print_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTEXSTRAOPTIONS:
		if (ignore_system_lyxrc ||
		    print_extra_options != system_lyxrc.print_extra_options) {
			os << "\\print_extra_options \"" << print_extra_options
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTSPOOL_COMMAND:
		if (ignore_system_lyxrc ||
		    print_spool_command != system_lyxrc.print_spool_command) {
			os << "\\print_spool_command \"" << escapeCommand(print_spool_command)
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTSPOOL_PRINTERPREFIX:
		if (ignore_system_lyxrc ||
		    print_spool_printerprefix
		    != system_lyxrc.print_spool_printerprefix) {
			os << "\\print_spool_printerprefix \""
			   << print_spool_printerprefix << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTEVENPAGEFLAG:
		if (ignore_system_lyxrc ||
		    print_evenpage_flag != system_lyxrc.print_evenpage_flag) {
			os << "\\print_evenpage_flag \"" << print_evenpage_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTODDPAGEFLAG:
		if (ignore_system_lyxrc ||
		    print_oddpage_flag != system_lyxrc.print_oddpage_flag) {
			os << "\\print_oddpage_flag \"" << print_oddpage_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTREVERSEFLAG:
		if (ignore_system_lyxrc ||
		    print_reverse_flag != system_lyxrc.print_reverse_flag) {
			os << "\\print_reverse_flag \"" << print_reverse_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTLANDSCAPEFLAG:
		if (ignore_system_lyxrc ||
		    print_landscape_flag != system_lyxrc.print_landscape_flag) {
			os << "\\print_landscape_flag \"" << print_landscape_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTPAGERANGEFLAG:
		if (ignore_system_lyxrc ||
		    print_pagerange_flag != system_lyxrc.print_pagerange_flag) {
			os << "\\print_pagerange_flag \"" << print_pagerange_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTCOPIESFLAG:
		if (ignore_system_lyxrc ||
		    print_copies_flag != system_lyxrc.print_copies_flag) {
			os << "\\print_copies_flag \"" << print_copies_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTCOLLCOPIESFLAG:
		if (ignore_system_lyxrc ||
		    print_collcopies_flag
		    != system_lyxrc.print_collcopies_flag) {
			os << "\\print_collcopies_flag \""
			   << print_collcopies_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTPAPERFLAG:
		if (ignore_system_lyxrc ||
		    print_paper_flag != system_lyxrc.print_paper_flag) {
			os << "\\print_paper_flag \"" << print_paper_flag
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTPAPERDIMENSIONFLAG:
		if (ignore_system_lyxrc ||
		    print_paper_dimension_flag
		    != system_lyxrc.print_paper_dimension_flag) {
			os << "\\print_paper_dimension_flag \""
			   << print_paper_dimension_flag << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTTOPRINTER:
		if (ignore_system_lyxrc ||
		    print_to_printer != system_lyxrc.print_to_printer) {
			os << "\\print_to_printer \"" << print_to_printer
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTTOFILE:
		if (ignore_system_lyxrc ||
		    print_to_file != system_lyxrc.print_to_file) {
			string const path = os::external_path(print_to_file);
			os << "\\print_to_file \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_PRINTFILEEXTENSION:
		if (ignore_system_lyxrc ||
		    print_file_extension != system_lyxrc.print_file_extension) {
			os << "\\print_file_extension \""
			   << print_file_extension
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# EXPORT SECTION ####################################\n"
		   << "#\n\n";

	case RC_CUSTOM_EXPORT_COMMAND:
		if (ignore_system_lyxrc ||
		    custom_export_command
		    != system_lyxrc.custom_export_command) {
			os << "\\custom_export_command \""
			   << custom_export_command
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_CUSTOM_EXPORT_FORMAT:
		if (ignore_system_lyxrc ||
		    custom_export_format
		    != system_lyxrc.custom_export_format) {
			os << "\\custom_export_format \"" << custom_export_format
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# TEX SECTION #######################################\n"
		   << "#\n\n";

	case RC_FONT_ENCODING:
		if (ignore_system_lyxrc ||
		    fontenc != system_lyxrc.fontenc) {
			os << "\\font_encoding \"" << fontenc << "\"\n";
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# FILE SECTION ######################################\n"
		   << "#\n\n";

	case RC_DOCUMENTPATH:
		if (ignore_system_lyxrc ||
		    document_path != system_lyxrc.document_path) {
			string const path = os::external_path(document_path);
			os << "\\document_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_USELASTFILEPOS:
		if (ignore_system_lyxrc ||
		    use_lastfilepos != system_lyxrc.use_lastfilepos) {
			os << "\\use_lastfilepos " << convert<string>(use_lastfilepos)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_LOADSESSION:
		if (ignore_system_lyxrc ||
		    load_session != system_lyxrc.load_session) {
			os << "\\load_session " << convert<string>(load_session)
			   << "\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_MOUSE_WHEEL_SPEED:
		if (ignore_system_lyxrc ||
		    mouse_wheel_speed != system_lyxrc.mouse_wheel_speed) {
			os << "\\mouse_wheel_speed " << mouse_wheel_speed << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_INLINE_DELAY:
		if (ignore_system_lyxrc ||
		    completion_inline_delay != system_lyxrc.completion_inline_delay) {
			os << "\\completion_inline_delay " << completion_inline_delay << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_INLINE_MATH:
		if (ignore_system_lyxrc ||
		    completion_inline_math != system_lyxrc.completion_inline_math) {
			os << "\\completion_inline_math "
				<< convert<string>(completion_inline_math) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_INLINE_TEXT:
		if (ignore_system_lyxrc ||
		    completion_inline_text != system_lyxrc.completion_inline_text) {
			os << "\\completion_inline_text "
				<< convert<string>(completion_inline_text) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_INLINE_DOTS:
		if (ignore_system_lyxrc ||
		    completion_inline_dots != system_lyxrc.completion_inline_dots) {
			os << "\\completion_inline_dots "
				<< convert<string>(completion_inline_dots) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_POPUP_DELAY:
		if (ignore_system_lyxrc ||
		    completion_popup_delay != system_lyxrc.completion_popup_delay) {
			os << "\\completion_popup_delay " << completion_popup_delay << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_POPUP_MATH:
		if (ignore_system_lyxrc ||
		    completion_popup_math != system_lyxrc.completion_popup_math) {
			os << "\\completion_popup_math "
				<< convert<string>(completion_popup_math) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_POPUP_TEXT:
		if (ignore_system_lyxrc ||
		    completion_popup_text != system_lyxrc.completion_popup_text) {
			os << "\\completion_popup_text "
				<< convert<string>(completion_popup_text) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_CURSOR_TEXT:
		if (ignore_system_lyxrc ||
		    completion_cursor_text != system_lyxrc.completion_cursor_text) {
			os << "\\completion_cursor_text "
			   << convert<string>(completion_cursor_text) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_COMPLETION_POPUP_AFTER_COMPLETE:
		if (ignore_system_lyxrc ||
		    completion_popup_after_complete
		    != system_lyxrc.completion_popup_after_complete) {
			os << "\\completion_popup_after_complete "
				<< convert<string>(completion_popup_after_complete) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_NUMLASTFILES:
		if (ignore_system_lyxrc ||
		    num_lastfiles != system_lyxrc.num_lastfiles) {
			os << "\\num_lastfiles " << num_lastfiles << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_CHECKLASTFILES:
		if (ignore_system_lyxrc ||
		    check_lastfiles != system_lyxrc.check_lastfiles) {
			os << "\\check_lastfiles " << convert<string>(check_lastfiles)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_EXAMPLEPATH:
		if (ignore_system_lyxrc ||
		    example_path != system_lyxrc.example_path) {
			string const path = os::external_path(example_path);
			os << "\\example_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_TEMPLATEPATH:
		if (ignore_system_lyxrc ||
		    template_path != system_lyxrc.template_path) {
			string const path = os::external_path(template_path);
			os << "\\template_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_TEMPDIRPATH:
		if (ignore_system_lyxrc ||
		    tempdir_path != system_lyxrc.tempdir_path) {
			string const path = os::external_path(tempdir_path);
			os << "\\tempdir_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_USETEMPDIR:
		if (tag != RC_LAST)
			break;
		// Ignore it
	case RC_PLAINTEXT_LINELEN:
		if (ignore_system_lyxrc ||
		    plaintext_linelen != system_lyxrc.plaintext_linelen) {
			os << "\\plaintext_linelen " << plaintext_linelen << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_MAKE_BACKUP:
		if (ignore_system_lyxrc ||
		    make_backup != system_lyxrc.make_backup) {
			os << "\\make_backup " << convert<string>(make_backup) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_BACKUPDIR_PATH:
		if (ignore_system_lyxrc ||
		    backupdir_path != system_lyxrc.backupdir_path) {
			string const path = os::external_path(backupdir_path);
			os << "\\backupdir_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# PLAIN TEXT EXPORT SECTION ##############################\n"
		   << "#\n\n";

	case RC_PLAINTEXT_ROFF_COMMAND:
		if (ignore_system_lyxrc ||
		    plaintext_roff_command != system_lyxrc.plaintext_roff_command) {
			os << "\\plaintext_roff_command \"" << escapeCommand(plaintext_roff_command)
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# SPELLCHECKER SECTION ##############################\n"
		   << "#\n\n";
	case RC_USE_SPELL_LIB:
		if (ignore_system_lyxrc ||
		    use_spell_lib != system_lyxrc.use_spell_lib) {
			os << "\\use_spell_lib " << convert<string>(use_spell_lib) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SPELL_COMMAND:
		if (ignore_system_lyxrc ||
		    isp_command != system_lyxrc.isp_command) {
			os << "\\spell_command \"" << escapeCommand(isp_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_ACCEPT_COMPOUND:
		if (ignore_system_lyxrc ||
		    isp_accept_compound != system_lyxrc.isp_accept_compound) {
			os << "\\accept_compound " << convert<string>(isp_accept_compound)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_ALT_LANG:
		if (ignore_system_lyxrc ||
		    isp_use_alt_lang != system_lyxrc.isp_use_alt_lang) {
			os << "\\use_alt_language " << convert<string>(isp_use_alt_lang)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_ALT_LANG:
		if (ignore_system_lyxrc ||
		    isp_alt_lang != system_lyxrc.isp_alt_lang) {
			os << "\\alternate_language \"" << isp_alt_lang
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_ESC_CHARS:
		if (ignore_system_lyxrc ||
		    isp_use_esc_chars != system_lyxrc.isp_use_esc_chars) {
			os << "\\use_escape_chars " << convert<string>(isp_use_esc_chars)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_ESC_CHARS:
		if (ignore_system_lyxrc ||
		    isp_esc_chars != system_lyxrc.isp_esc_chars) {
			os << "\\escape_chars \"" << isp_esc_chars << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_PERS_DICT:
		if (ignore_system_lyxrc ||
		    isp_use_pers_dict != system_lyxrc.isp_use_pers_dict) {
			os << "\\use_personal_dictionary "
			   << convert<string>(isp_use_pers_dict)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_TOOLTIP:
		if (ignore_system_lyxrc ||
		    use_tooltip != system_lyxrc.use_tooltip) {
			os << "\\use_tooltip "
			   << convert<string>(use_tooltip)
			   << '\n';
		}
	case RC_USE_PIXMAP_CACHE:
		if (ignore_system_lyxrc ||
		    use_pixmap_cache != system_lyxrc.use_pixmap_cache) {
			os << "\\use_pixmap_cache "
			   << convert<string>(use_pixmap_cache)
			   << '\n';
		}
	case RC_PERS_DICT:
		if (isp_pers_dict != system_lyxrc.isp_pers_dict) {
			string const path = os::external_path(isp_pers_dict);
			os << "\\personal_dictionary \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_INP_ENC:
		if (ignore_system_lyxrc ||
		    isp_use_input_encoding
		    != system_lyxrc.isp_use_input_encoding) {
			os << "\\use_input_encoding "
			   << convert<string>(isp_use_input_encoding)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# LANGUAGE SUPPORT SECTION ##########################\n"
		   << "#\n\n";

	case RC_RTL_SUPPORT:
		if (ignore_system_lyxrc ||
		    rtl_support != system_lyxrc.rtl_support) {
			os << "\\rtl " << convert<string>(rtl_support) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_VISUAL_CURSOR:
		if (ignore_system_lyxrc ||
			visual_cursor != system_lyxrc.visual_cursor) {
			os << "\\visual_cursor " << convert<string>(visual_cursor) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_PACKAGE:
		if (ignore_system_lyxrc ||
		    language_package != system_lyxrc.language_package) {
			os << "\\language_package \"" << language_package
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_GLOBAL_OPTIONS:
		if (ignore_system_lyxrc ||
		    language_global_options
		    != system_lyxrc.language_global_options) {
			os << "\\language_global_options \""
			   << convert<string>(language_global_options)
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_USE_BABEL:
		if (ignore_system_lyxrc ||
		    language_use_babel != system_lyxrc.language_use_babel) {
			os << "\\language_use_babel \""
			   << convert<string>(language_use_babel)
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_COMMAND_BEGIN:
		if (ignore_system_lyxrc ||
		    language_command_begin
		    != system_lyxrc.language_command_begin) {
			os << "\\language_command_begin \""
			   << language_command_begin
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_COMMAND_END:
		if (ignore_system_lyxrc ||
		    language_command_end
		    != system_lyxrc.language_command_end) {
			os << "\\language_command_end \"" << language_command_end
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_COMMAND_LOCAL:
		if (ignore_system_lyxrc ||
		    language_command_local
		    != system_lyxrc.language_command_local) {
			os << "\\language_command_local \""
			   << language_command_local
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_AUTO_BEGIN:
		if (ignore_system_lyxrc ||
		    language_auto_begin != system_lyxrc.language_auto_begin) {
			os << "\\language_auto_begin "
			   << convert<string>(language_auto_begin) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_AUTO_END:
		if (ignore_system_lyxrc ||
		    language_auto_end != system_lyxrc.language_auto_end) {
			os << "\\language_auto_end "
			   << convert<string>(language_auto_end) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_MARK_FOREIGN_LANGUAGE:
		if (ignore_system_lyxrc ||
		    mark_foreign_language
		    != system_lyxrc.mark_foreign_language) {
			os << "\\mark_foreign_language " <<
				convert<string>(mark_foreign_language) << '\n';
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# 2nd MISC SUPPORT SECTION ##########################\n"
		   << "#\n\n";

	case RC_AUTO_NUMBER:
		if (ignore_system_lyxrc ||
		    auto_number != system_lyxrc.auto_number) {
			os << "\\auto_number " << convert<string>(auto_number) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_DEFAULT_LANGUAGE:
		if (ignore_system_lyxrc ||
		    default_language != system_lyxrc.default_language) {
			os << "\\default_language " << default_language << '\n';
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# FORMATS SECTION ##########################\n"
		   << "#\n\n";

	case RC_FORMAT:
		// New/modified formats
		for (Formats::const_iterator cit = formats.begin();
		     cit != formats.end(); ++cit) {
			Format const * format =
				system_formats.getFormat(cit->name());
			if (!format ||
			    format->extension() != cit->extension() ||
			    format->prettyname() != cit->prettyname() ||
			    format->shortcut() != cit->shortcut() ||
			    format->viewer() != cit->viewer() ||
			    format->editor() != cit->editor() ||
			    format->documentFormat() != cit->documentFormat() ||
			    format->vectorFormat() != cit->vectorFormat()) {
				os << "\\format \"" << cit->name() << "\" \""
				   << cit->extension() << "\" \""
				   << cit->prettyname() << "\" \""
				   << cit->shortcut() << "\" \""
				   << escapeCommand(cit->viewer()) << "\" \""
				   << escapeCommand(cit->editor()) << "\" \"";
				vector<string> flags;
				if (cit->documentFormat())
					flags.push_back("document");
				if (cit->vectorFormat())
					flags.push_back("vector");
				os << getStringFromVector(flags);
				os << "\"\n";
			}
		}

		// Look for deleted formats
		for (Formats::const_iterator cit = system_formats.begin();
		     cit != system_formats.end(); ++cit)
			if (!formats.getFormat(cit->name()))
				os << "\\format \"" << cit->name()
				   << "\" \"\" \"\" \"\" \"\" \"\" \"\"\n";
		if (tag != RC_LAST)
			break;
	case RC_VIEWER:
		// Ignore it
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# CONVERTERS SECTION ##########################\n"
		   << "#\n\n";

	case RC_CONVERTER:
		// Look for new converters
		for (Converters::const_iterator cit = theConverters().begin();
		     cit != theConverters().end(); ++cit) {
			Converter const * converter =
				theSystemConverters().getConverter(cit->from,
							       cit->to);
			if (!converter ||
			    converter->command != cit->command ||
			    converter->flags != cit->flags)
				os << "\\converter \"" << cit->from << "\" \""
				   << cit->to << "\" \""
				   << escapeCommand(cit->command) << "\" \""
				   << cit->flags << "\"\n";
		}

		// New/modifed converters
		for (Converters::const_iterator cit = theSystemConverters().begin();
		     cit != theSystemConverters().end(); ++cit)
			if (!theConverters().getConverter(cit->from, cit->to))
				os << "\\converter \"" << cit->from
				   << "\" \"" << cit->to << "\" \"\" \"\"\n";
		if (tag != RC_LAST)
			break;

	case RC_COPIER:
		if (tag == RC_LAST)
			os << "\n#\n"
			   << "# COPIERS SECTION ##########################\n"
			   << "#\n\n";

		// Look for new movers
		Movers::const_iterator const sysbegin = theSystemMovers().begin();
		Movers::const_iterator const sysend = theSystemMovers().end();
		Movers::const_iterator it = theMovers().begin();
		Movers::const_iterator end = theMovers().end();

		for (; it != end; ++it) {
			Movers::const_iterator const sysit =
				find_if(sysbegin, sysend, SameMover(*it));
			if (sysit == sysend) {
				string const & fmt = it->first;
				string const & command =
					it->second.command();

				os << "\\copier " << fmt
				   << " \"" << escapeCommand(command) << "\"\n";
			}
		}
		if (tag != RC_LAST)
			break;

		// We don't actually delete SpecialisedMover(s) from the
		// map, just clear their 'command', so there's no need
		// to test for anything else.
	}

	os.flush();
}


#if 0
string const LyXRC::getDescription(LyXRCTags tag)
{
	docstring str;

	switch (tag) {
	case RC_ACCEPT_COMPOUND:
		str = _("Consider run-together words, such as \"diskdrive\" for \"disk drive\", as legal words?");
		break;

	case RC_ALT_LANG:
	case RC_USE_ALT_LANG:
		str = _("Specify an alternate language. The default is to use the language of the document.");
		break;

	case RC_PLAINTEXT_ROFF_COMMAND:
		str = _("Use to define an external program to render tables in plain text output. E.g. \"groff -t -Tlatin1 $$FName\" where $$FName is the input file. If \"\" is specified, an internal routine is used.");
		break;

	case RC_PLAINTEXT_LINELEN:
		str = _("The maximum line length of exported plain text/LaTeX/SGML files. If set to 0, paragraphs are output in a single line; if the line length is > 0, paragraphs are separated by a blank line.");
		break;

	case RC_AUTOREGIONDELETE:
		str = _("De-select if you don't want the current selection to be replaced automatically by what you type.");
		break;

	case RC_AUTORESET_OPTIONS:
		str = _("De-select if you don't want the class options to be reset to defaults after class change.");
		break;

	case RC_AUTOSAVE:
		str = _("The time interval between auto-saves (in seconds). 0 means no auto-save.");
		break;

	case RC_AUTO_NUMBER:
		break;

	case RC_BACKUPDIR_PATH:
		str = _("The path for storing backup files. If it is an empty string, LyX will store the backup file in the same directory as the original file.");
		break;

	case RC_BIBTEX_COMMAND:
		str = _("Define the options of bibtex (cf. man bibtex) or select an alternative compiler (e.g. mlbibtex or bibulus).");
		break;

	case RC_BINDFILE:
		str = _("Keybindings file. Can either specify an absolute path, or LyX will look in its global and local bind/ directories.");
		break;

	case RC_CHECKLASTFILES:
		str = _("Select to check whether the lastfiles still exist.");
		break;

	case RC_CHKTEX_COMMAND:
		str = _("Define how to run chktex. E.g. \"chktex -n11 -n1 -n3 -n6 -n9 -22 -n25 -n30 -n38\" Refer to the ChkTeX documentation.");
		break;

	case RC_CONVERTER:
		break;

	case RC_COPIER:
		break;

	case RC_CURSOR_FOLLOWS_SCROLLBAR:
		str = _("LyX normally doesn't update the cursor position if you move the scrollbar. Set to true if you'd prefer to always have the cursor on screen.");
		break;

	case RC_SHOW_MACRO_LABEL:
		str = _("Show a small box around a Math Macro with the macro name when the cursor is inside.");
		break;

	case RC_CUSTOM_EXPORT_COMMAND:
		break;

	case RC_CUSTOM_EXPORT_FORMAT:
		break;

	case RC_DATE_INSERT_FORMAT:
		//xgettext:no-c-format
		str = _("This accepts the normal strftime formats; see man strftime for full details. E.g.\"%A, %e. %B %Y\".");
		break;

	case RC_DEFFILE:
		str = _("Command definition file. Can either specify an absolute path, or LyX will look in its global and local commands/ directories.");
		break;

	case RC_DEFAULT_LANGUAGE:
		str = _("New documents will be assigned this language.");
		break;

	case RC_DEFAULT_PAPERSIZE:
		str = _("Specify the default paper size.");
		break;

	case RC_DIALOGS_ICONIFY_WITH_MAIN:
		str = _("Iconify the dialogs when the main window is iconified. (Affects only dialogs shown after the change has been made.)");
		break;

	case RC_DISPLAY_GRAPHICS:
		str = _("Select how LyX will display any graphics.");
		break;

	case RC_DOCUMENTPATH:
		str = _("The default path for your documents. An empty value selects the directory LyX was started from.");
		break;

	case RC_ESC_CHARS:
	case RC_USE_ESC_CHARS:
		str = _("Specify additional chars that can be part of a word.");
		break;

	case RC_EXAMPLEPATH:
		str = _("The path that LyX will set when offering to choose an example. An empty value selects the directory LyX was started from.");
		break;

	case RC_FONT_ENCODING:
		str = _("The font encoding used for the LaTeX2e fontenc package. T1 is highly recommended for non-English languages.");
		break;

	case RC_FORMAT:
		break;

	case RC_INDEX_COMMAND:
		str = _("Define the options of makeindex (cf. man makeindex) or select an alternative compiler. E.g., using xindy/make-rules, the command string would be \"makeindex.sh -m $$lang\".");
		break;

	case RC_INPUT:
		break;

	case RC_KBMAP:
	case RC_KBMAP_PRIMARY:
	case RC_KBMAP_SECONDARY:
		str = _("Use this to set the correct mapping file for your keyboard. You'll need this if you for instance want to type German documents on an American keyboard.");
		break;

	case RC_LABEL_INIT_LENGTH:
		str = _("Maximum number of words in the initialization string for a new label");
		break;

	case RC_LANGUAGE_AUTO_BEGIN:
		str = _("Select if a language switching command is needed at the beginning of the document.");
		break;

	case RC_LANGUAGE_AUTO_END:
		str = _("Select if a language switching command is needed at the end of the document.");
		break;

	case RC_LANGUAGE_COMMAND_BEGIN:
		str = _("The LaTeX command for changing from the language of the document to another language. E.g. \\selectlanguage{$$lang} where $$lang is substituted by the name of the second language.");
		break;

	case RC_LANGUAGE_COMMAND_END:
		str = _("The LaTeX command for changing back to the language of the document.");
		break;

	case RC_LANGUAGE_COMMAND_LOCAL:
		str = _("The LaTeX command for local changing of the language.");
		break;

	case RC_LANGUAGE_GLOBAL_OPTIONS:
		str = _("De-select if you don't want the language(s) used as an argument to \\documentclass.");
		break;

	case RC_LANGUAGE_PACKAGE:
		str = _("The LaTeX command for loading the language package. E.g. \"\\usepackage{babel}\", \"\\usepackage{omega}\".");
		break;

	case RC_LANGUAGE_USE_BABEL:
		str = _("De-select if you don't want babel to be used when the language of the document is the default language.");
		break;

	case RC_USELASTFILEPOS:
		str = _("De-select if you do not want LyX to scroll to saved position.");
		break;

	case RC_LOADSESSION:
		str = _("De-select to prevent loading files opened from the last LyX session.");
		break;

	case RC_MAKE_BACKUP:
		str = _("De-select if you don't want LyX to create backup files.");
		break;

	case RC_MARK_FOREIGN_LANGUAGE:
		str = _("Select to control the highlighting of words with a language foreign to that of the document.");
		break;

	case RC_MOUSE_WHEEL_SPEED:
		str = bformat(_("The scrolling speed of the mouse wheel."),
		      maxlastfiles);
		break;

	case RC_COMPLETION_POPUP_DELAY:
		str = _("The completion popup delay.");
		break;

	case RC_COMPLETION_POPUP_MATH:
		str = _("Select to display the completion popup in math mode.");
		break;

	case RC_COMPLETION_POPUP_TEXT:
		str = _("Select to display the completion popup in text mode.");
		break;

	case RC_COMPLETION_POPUP_AFTER_COMPLETE:
		str = _("Show the completion popup without delay after non-unique completion attempt.");
		break;

	case RC_COMPLETION_POPUP_TEXT:
		str = _("Show a small triangle on the cursor to indicate that a completion is available.");
		break;

	case RC_COMPLETION_POPUP_DELAY:
		str = _("The inline completion delay.");
		break;

	case RC_COMPLETION_INLINE_MATH:
		str = _("Select to display the inline completion in math mode.");
		break;

	case RC_COMPLETION_INLINE_TEXT:
		str = _("Select to display the inline completion in text mode.");
		break;

	case RC_COMPLETION_INLINE_DOTS:
		str = _("Use \"...\" to shorten long completions.");
		break;

	case RC_NUMLASTFILES:
		str = bformat(_("Maximal number of lastfiles. Up to %1$d can appear in the file menu."),
			maxlastfiles);
		break;

	case RC_PATH_PREFIX:
		str = _("Specify those directories which should be "
			 "prepended to the PATH environment variable. "
			 "Use the OS native format.");
		break;

	case RC_PERS_DICT:
	case RC_USE_PERS_DICT:
		str = _("Specify an alternate personal dictionary file. E.g. \".ispell_english\".");
		break;

	case RC_PREVIEW:
		str = _("Shows a typeset preview of things such as math");
		break;

	case RC_PREVIEW_HASHED_LABELS:
		str = _("Previewed equations will have \"(#)\" labels rather than numbered ones");
		break;

	case RC_PREVIEW_SCALE_FACTOR:
		str = _("Scale the preview size to suit.");
		break;

	case RC_PRINTCOLLCOPIESFLAG:
		str = _("The option for specifying whether the copies should be collated.");
		break;

	case RC_PRINTCOPIESFLAG:
		str = _("The option for specifying the number of copies to print.");
		break;

	case RC_PRINTER:
		str = _("The default printer to print on. If none is specified, LyX will use the environment variable PRINTER.");
		break;

	case RC_PRINTEVENPAGEFLAG:
		str = _("The option to print only even pages.");
		break;

	case RC_PRINTEXSTRAOPTIONS:
		str = _("Extra options to pass to printing program after everything else, but before the filename of the DVI file to be printed.");
		break;

	case RC_PRINTFILEEXTENSION:
		str = _("Extension of printer program output file. Usually \".ps\".");
		break;

	case RC_PRINTLANDSCAPEFLAG:
		str = _("The option to print out in landscape.");
		break;

	case RC_PRINTODDPAGEFLAG:
		str = _("The option to print only odd pages.");
		break;

	case RC_PRINTPAGERANGEFLAG:
		str = _("The option for specifying a comma-separated list of pages to print.");
		break;

	case RC_PRINTPAPERDIMENSIONFLAG:
				   str = _("Option to specify the dimensions of the print paper.");
		break;

	case RC_PRINTPAPERFLAG:
		str = _("The option to specify paper type.");
		break;

	case RC_PRINTREVERSEFLAG:
		str = _("The option to reverse the order of the pages printed.");
		break;

	case RC_PRINTSPOOL_COMMAND:
		str = _("When set, this printer option automatically prints to a file and then calls a separate print spooling program on that file with the given name and arguments.");
		break;

	case RC_PRINTSPOOL_PRINTERPREFIX:
		str = _("If you specify a printer name in the print dialog, the following argument is prepended along with the printer name after the spool command.");
		break;

	case RC_PRINTTOFILE:
		str = _("Option to pass to the print program to print to a file.");
		break;

	case RC_PRINTTOPRINTER:
		str = _("Option to pass to the print program to print on a specific printer.");
		break;

	case RC_PRINT_ADAPTOUTPUT:
		str = _("Select for LyX to pass the name of the destination printer to your print command.");
		break;

	case RC_PRINT_COMMAND:
		str = _("Your favorite print program, e.g. \"dvips\", \"dvilj4\".");
		break;

	case RC_RTL_SUPPORT:
		str = _("Select to enable support of right-to-left languages (e.g. Hebrew, Arabic).");
		break;

	case RC_VISUAL_CURSOR:
		str = _("Select to have visual bidi cursor movement, unselect for logical movement.");
		break;

	case RC_SCREEN_DPI:
		str = _("DPI (dots per inch) of your monitor is auto-detected by LyX. If that goes wrong, override the setting here.");
		break;

	case RC_SCREEN_FONT_ROMAN:
	case RC_SCREEN_FONT_SANS:
	case RC_SCREEN_FONT_TYPEWRITER:
		str = _("The screen fonts used to display the text while editing.");
		break;

	case RC_SCREEN_FONT_ROMAN_FOUNDRY:
	case RC_SCREEN_FONT_SANS_FOUNDRY:
	case RC_SCREEN_FONT_TYPEWRITER_FOUNDRY:
		break;

	case RC_SCREEN_FONT_SCALABLE:
		str = _("Allow bitmap fonts to be resized. If you are using a bitmap font, selecting this option may make some fonts look blocky in LyX. Deselecting this option makes LyX use the nearest bitmap font size available, instead of scaling.");
		break;

	case RC_SCREEN_FONT_SIZES:
		str = _("The font sizes used for calculating the scaling of the screen fonts.");
		break;

	case RC_SCREEN_ZOOM:
		//xgettext:no-c-format
		str = _("The zoom percentage for screen fonts. A setting of 100% will make the fonts roughly the same size as on paper.");
		break;

	case RC_GEOMETRY_SESSION:
		str = _("Allow session manager to save and restore windows geometry.");
		break;

	case RC_SERVERPIPE:
		str = _("This starts the lyxserver. The pipes get an additional extension \".in\" and \".out\". Only for advanced users.");
		break;

	case RC_SET_COLOR:
		break;

	case RC_SHOW_BANNER:
		str = _("De-select if you don't want the startup banner.");
		break;

	case RC_SPELL_COMMAND:
		str = _("What command runs the spellchecker?");
		break;

	case RC_TEMPDIRPATH:
		str = _("LyX will place its temporary directories in this path. They will be deleted when you quit LyX.");
		break;

	case RC_TEMPLATEPATH:
		str = _("The path that LyX will set when offering to choose a template. An empty value selects the directory LyX was started from.");
		break;

	case RC_TEX_ALLOWS_SPACES:
		break;

	case RC_TEX_EXPECTS_WINDOWS_PATHS:
		break;

	case RC_UIFILE:
		str = _("The UI (user interface) file. Can either specify an absolute path, or LyX will look in its global and local ui/ directories.");
		break;

	case RC_USER_EMAIL:
		break;

	case RC_USER_NAME:
		break;

	case RC_USETEMPDIR:
		break;

	case RC_USE_INP_ENC:
		str = _("Specify whether to pass the -T input encoding option to ispell. Enable this if you cannot check the spelling of words containing accented letters. This may not work with all dictionaries.");
		break;

	case RC_USE_TOOLTIP:
		str = _("Enable the automatic appearance of tool tips in the work area.");
		break;

	case RC_USE_PIXMAP_CACHE:
		str = _("Enable the pixmap cache that might improve performance on Mac and Windows.");
		break;

	case RC_USE_SPELL_LIB:
		break;

	case RC_VIEWDVI_PAPEROPTION:
		_("Specify the paper command to DVI viewer (leave empty or use \"-paper\")");
		break;

	case RC_VIEWER:
		break;

	case RC_LAST:
		break;
	}

	return str;
}
#endif


// The global instance
LyXRC lyxrc;

// The global copy of the system lyxrc entries (everything except preferences)
LyXRC system_lyxrc;


} // namespace lyx
