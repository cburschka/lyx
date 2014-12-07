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

#include "LyXRC.h"

#include "ColorSet.h"
#include "Converter.h"
#include "FontEnums.h"
#include "Format.h"
#include "Lexer.h"
#include "LyX.h"
#include "Mover.h"
#include "Session.h"
#include "SpellChecker.h"
#include "version.h"

#include "graphics/GraphicsTypes.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/environment.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/TempFile.h"
#include "support/userinfo.h"

#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace os = support::os;

namespace {

// The format should also be updated in configure.py, and conversion code
// should be added to prefs2prefs_prefs.py.
static unsigned int const LYXRC_FILEFORMAT = 17; // lasgouttes: remove \\rtl

// when adding something to this array keep it sorted!
LexerKeyword lyxrcTags[] = {
	{ "\\accept_compound", LyXRC::RC_ACCEPT_COMPOUND },
	{ "\\allow_geometry_session", LyXRC::RC_GEOMETRY_SESSION },
	{ "\\alternate_language", LyXRC::RC_ALT_LANG },
	{ "\\auto_number", LyXRC::RC_AUTO_NUMBER },
	{ "\\auto_region_delete", LyXRC::RC_AUTOREGIONDELETE },
	{ "\\auto_reset_options", LyXRC::RC_AUTORESET_OPTIONS },
	{ "\\autocorrection_math", LyXRC::RC_AUTOCORRECTION_MATH },
	{ "\\autosave", LyXRC::RC_AUTOSAVE },
	{ "\\backupdir_path", LyXRC::RC_BACKUPDIR_PATH },
	{ "\\bibtex_alternatives", LyXRC::RC_BIBTEX_ALTERNATIVES },
	{ "\\bibtex_command", LyXRC::RC_BIBTEX_COMMAND },
	{ "\\bind_file", LyXRC::RC_BINDFILE },
	{ "\\check_lastfiles", LyXRC::RC_CHECKLASTFILES },
	{ "\\chktex_command", LyXRC::RC_CHKTEX_COMMAND },
	{ "\\close_buffer_with_last_view", LyXRC::RC_CLOSE_BUFFER_WITH_LAST_VIEW },
	{ "\\completion_cursor_text", LyXRC::RC_COMPLETION_CURSOR_TEXT },
	{ "\\completion_inline_delay", LyXRC::RC_COMPLETION_INLINE_DELAY },
	{ "\\completion_inline_dots", LyXRC::RC_COMPLETION_INLINE_DOTS },
	{ "\\completion_inline_math", LyXRC::RC_COMPLETION_INLINE_MATH },
	{ "\\completion_inline_text", LyXRC::RC_COMPLETION_INLINE_TEXT },
	{ "\\completion_minlength", LyXRC::RC_COMPLETION_MINLENGTH },
	{ "\\completion_popup_after_complete", LyXRC::RC_COMPLETION_POPUP_AFTER_COMPLETE },
	{ "\\completion_popup_delay", LyXRC::RC_COMPLETION_POPUP_DELAY },
	{ "\\completion_popup_math", LyXRC::RC_COMPLETION_POPUP_MATH },
	{ "\\completion_popup_text", LyXRC::RC_COMPLETION_POPUP_TEXT },
	{ "\\converter", LyXRC::RC_CONVERTER },
	{ "\\converter_cache_maxage", LyXRC::RC_CONVERTER_CACHE_MAXAGE },
	{ "\\copier", LyXRC::RC_COPIER },
	{ "\\cursor_follows_scrollbar", LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR },
	{ "\\cursor_width", LyXRC::RC_CURSOR_WIDTH },
	{ "\\date_insert_format", LyXRC::RC_DATE_INSERT_FORMAT },
	{ "\\def_file", LyXRC::RC_DEFFILE },
	{ "\\default_decimal_point", LyXRC::RC_DEFAULT_DECIMAL_POINT },
	{ "\\default_length_unit", LyXRC::RC_DEFAULT_LENGTH_UNIT },
	{ "\\default_otf_view_format", LyXRC::RC_DEFAULT_OTF_VIEW_FORMAT },
	{ "\\default_view_format", LyXRC::RC_DEFAULT_VIEW_FORMAT },
	{ "\\dialogs_iconify_with_main", LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN },
	{ "\\display_graphics", LyXRC::RC_DISPLAY_GRAPHICS },
	{ "\\document_path", LyXRC::RC_DOCUMENTPATH },
	{ "\\editor_alternatives", LyXRC::RC_EDITOR_ALTERNATIVES },
	{ "\\escape_chars", LyXRC::RC_ESC_CHARS },
	{ "\\example_path", LyXRC::RC_EXAMPLEPATH },
	{ "\\export_overwrite", LyXRC::RC_EXPORT_OVERWRITE },
	{ "\\font_encoding", LyXRC::RC_FONT_ENCODING },
	{ "\\format", LyXRC::RC_FILEFORMAT },
	{ "\\forward_search_dvi", LyXRC::RC_FORWARD_SEARCH_DVI },
	{ "\\forward_search_pdf", LyXRC::RC_FORWARD_SEARCH_PDF },
	{ "\\fullscreen_limit", LyXRC::RC_FULL_SCREEN_LIMIT },
	{ "\\fullscreen_menubar", LyXRC::RC_FULL_SCREEN_MENUBAR },
	{ "\\fullscreen_scrollbar", LyXRC::RC_FULL_SCREEN_SCROLLBAR },
	{ "\\fullscreen_statusbar", LyXRC::RC_FULL_SCREEN_STATUSBAR },
	{ "\\fullscreen_tabbar", LyXRC::RC_FULL_SCREEN_TABBAR },
	{ "\\fullscreen_toolbars", LyXRC::RC_FULL_SCREEN_TOOLBARS },
	{ "\\fullscreen_width", LyXRC::RC_FULL_SCREEN_WIDTH },
	{ "\\group_layouts", LyXRC::RC_GROUP_LAYOUTS },
	{ "\\gui_language", LyXRC::RC_GUI_LANGUAGE },
	{ "\\hunspelldir_path", LyXRC::RC_HUNSPELLDIR_PATH },
	{ "\\icon_set", LyXRC::RC_ICON_SET },
	{ "\\index_alternatives", LyXRC::RC_INDEX_ALTERNATIVES },
	{ "\\index_command", LyXRC::RC_INDEX_COMMAND },
	{ "\\input", LyXRC::RC_INPUT },
	{ "\\jbibtex_command", LyXRC::RC_JBIBTEX_COMMAND },
	{ "\\jindex_command", LyXRC::RC_JINDEX_COMMAND },
	{ "\\kbmap", LyXRC::RC_KBMAP },
	{ "\\kbmap_primary", LyXRC::RC_KBMAP_PRIMARY },
	{ "\\kbmap_secondary", LyXRC::RC_KBMAP_SECONDARY },
	{ "\\language_auto_begin", LyXRC::RC_LANGUAGE_AUTO_BEGIN },
	{ "\\language_auto_end", LyXRC::RC_LANGUAGE_AUTO_END },
	{ "\\language_command_begin", LyXRC::RC_LANGUAGE_COMMAND_BEGIN },
	{ "\\language_command_end", LyXRC::RC_LANGUAGE_COMMAND_END },
	{ "\\language_command_local", LyXRC::RC_LANGUAGE_COMMAND_LOCAL },
	{ "\\language_custom_package", LyXRC::RC_LANGUAGE_CUSTOM_PACKAGE },
	{ "\\language_global_options", LyXRC::RC_LANGUAGE_GLOBAL_OPTIONS },
	{ "\\language_package_selection", LyXRC::RC_LANGUAGE_PACKAGE_SELECTION },
	{ "\\load_session", LyXRC::RC_LOADSESSION },
	{ "\\mac_dontswap_ctrl_meta", LyXRC::RC_MAC_DONTSWAP_CTRL_META },
	{ "\\mac_like_cursor_movement", LyXRC::RC_MAC_LIKE_CURSOR_MOVEMENT },
	{ "\\macro_edit_style", LyXRC::RC_MACRO_EDIT_STYLE },
	{ "\\make_backup", LyXRC::RC_MAKE_BACKUP },
	{ "\\mark_foreign_language", LyXRC::RC_MARK_FOREIGN_LANGUAGE },
	{ "\\mouse_wheel_speed", LyXRC::RC_MOUSE_WHEEL_SPEED },
	{ "\\nomencl_command", LyXRC::RC_NOMENCL_COMMAND },
	{ "\\num_lastfiles", LyXRC::RC_NUMLASTFILES },
	{ "\\open_buffers_in_tabs", LyXRC::RC_OPEN_BUFFERS_IN_TABS },
	{ "\\paragraph_markers", LyXRC::RC_PARAGRAPH_MARKERS },
	{ "\\path_prefix", LyXRC::RC_PATH_PREFIX },
	{ "\\plaintext_linelen", LyXRC::RC_PLAINTEXT_LINELEN },
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
	{ "\\save_compressed", LyXRC::RC_SAVE_COMPRESSED },
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
	{ "\\scroll_below_document", LyXRC::RC_SCROLL_BELOW_DOCUMENT },
	{ "\\scroll_wheel_zoom", LyXRC::RC_SCROLL_WHEEL_ZOOM },
	{ "\\serverpipe", LyXRC::RC_SERVERPIPE },
	{ "\\set_color", LyXRC::RC_SET_COLOR },
	{ "\\show_banner", LyXRC::RC_SHOW_BANNER },
	{ "\\single_close_tab_button", LyXRC::RC_SINGLE_CLOSE_TAB_BUTTON },
	{ "\\single_instance", LyXRC::RC_SINGLE_INSTANCE },
	{ "\\sort_layouts", LyXRC::RC_SORT_LAYOUTS },
	{ "\\spellcheck_continuously", LyXRC::RC_SPELLCHECK_CONTINUOUSLY },
	{ "\\spellcheck_notes", LyXRC::RC_SPELLCHECK_NOTES },
	{ "\\spellchecker", LyXRC::RC_SPELLCHECKER },
	{ "\\splitindex_command", LyXRC::RC_SPLITINDEX_COMMAND },
	{ "\\tempdir_path", LyXRC::RC_TEMPDIRPATH },
	{ "\\template_path", LyXRC::RC_TEMPLATEPATH },
	{ "\\tex_allows_spaces", LyXRC::RC_TEX_ALLOWS_SPACES },
	{ "\\tex_expects_windows_paths", LyXRC::RC_TEX_EXPECTS_WINDOWS_PATHS },
	{ "\\texinputs_prefix", LyXRC::RC_TEXINPUTS_PREFIX },
	{ "\\thesaurusdir_path", LyXRC::RC_THESAURUSDIRPATH },
	{ "\\ui_file", LyXRC::RC_UIFILE },
	{ "\\use_converter_cache", LyXRC::RC_USE_CONVERTER_CACHE },
	{ "\\use_lastfilepos", LyXRC::RC_USELASTFILEPOS },
	{ "\\use_pixmap_cache", LyXRC::RC_USE_PIXMAP_CACHE },
	{ "\\use_qimage", LyXRC::RC_USE_QIMAGE },
	// compatibility with versions older than 1.4.0 only
	{ "\\use_system_colors", LyXRC::RC_USE_SYSTEM_COLORS },
	{ "\\use_system_theme_icons", LyXRC::RC_USE_SYSTEM_THEME_ICONS },
	{ "\\use_tooltip", LyXRC::RC_USE_TOOLTIP },
	{ "\\user_email", LyXRC::RC_USER_EMAIL },
	{ "\\user_name", LyXRC::RC_USER_NAME },
	{ "\\view_dvi_paper_option", LyXRC::RC_VIEWDVI_PAPEROPTION },
	// compatibility with versions older than 1.4.0 only
	{ "\\viewer", LyXRC::RC_VIEWER},
	{ "\\viewer_alternatives", LyXRC::RC_VIEWER_ALTERNATIVES },
	{ "\\visual_cursor", LyXRC::RC_VISUAL_CURSOR },
	{ "format", LyXRC::RC_LYXRCFORMAT }
};

const int lyxrcCount = sizeof(lyxrcTags) / sizeof(lyxrcTags[0]);

} // namespace anon


LyXRC::LyXRC()
{
	setDefaults();
}


void LyXRC::setDefaults()
{
	icon_set = string();
	use_system_theme_icons = false;
	bind_file = "cua";
	def_file = "default";
	ui_file = "default";
	// The current document directory
	texinputs_prefix = ".";
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
	default_view_format = "pdf2";
	default_otf_view_format = "pdf4";
	chktex_command = "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38";
	bibtex_command = "bibtex";
	fontenc = "default";
	index_command = "makeindex -c -q";
	nomencl_command = "makeindex -s nomencl.ist";
	dpi = 75;
	// Because a screen is typically wider than a piece of paper:
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
	save_compressed = false;
	backupdir_path.erase();
	display_graphics = true;
	// Spellchecker settings:
// FIXME: this check should test the target platform (darwin)
#if defined(USE_MACOSX_PACKAGING)
	spellchecker = "native";
#elif defined(USE_ASPELL)
	spellchecker = "aspell";
#elif defined(USE_HUNSPELL)
	spellchecker = "hunspell";
#else
	spellchecker = "aspell";
#endif
	spellchecker_accept_compound = false;
	spellcheck_continuously = false;
	completion_minlength = 6;
	spellcheck_notes = true;
	use_kbmap = false;
	visual_cursor = false;
	auto_number = true;
	mark_foreign_language = true;
	language_auto_begin = true;
	language_auto_end = true;
	language_global_options = true;
	language_package_selection = LP_AUTO;
	language_custom_package = "\\usepackage{babel}";
	language_command_begin = "\\selectlanguage{$$lang}";
	language_command_local = "\\foreignlanguage{$$lang}{";
	sort_layouts = false;
	group_layouts = true;
	gui_language = "auto";
	show_banner = true;
	windows_style_tex_paths = false;
	tex_allows_spaces = false;
	date_insert_format = "%x";
	cursor_follows_scrollbar = false;
	scroll_below_document = false;
	scroll_wheel_zoom = SCROLL_WHEEL_ZOOM_CTRL;
	paragraph_markers = false;
	mac_dontswap_ctrl_meta = false;
	mac_like_cursor_movement = false;
	macro_edit_style = MACRO_EDIT_INLINE_BOX;
	dialogs_iconify_with_main = false;
	preview = PREVIEW_OFF;
	preview_hashed_labels  = false;
	preview_scale_factor = 1.0;
	use_converter_cache = true;
	use_system_colors = false;
	use_tooltip = true;
	use_pixmap_cache = false;
	use_qimage = true;
	converter_cache_maxage = 6 * 30 * 24 * 3600; // 6 months
	user_name = to_utf8(support::user_name());
	user_email = to_utf8(support::user_email());
	open_buffers_in_tabs = true;
	single_close_tab_button = false;
	single_instance = true;
	forward_search_dvi = string();
	forward_search_pdf = string();
	export_overwrite = NO_FILES;

	// Fullscreen settings
	full_screen_limit = false;
	full_screen_toolbars = true;
	full_screen_tabbar = true;
	full_screen_menubar = true;
	full_screen_scrollbar = true;
	full_screen_width = 700;

	completion_cursor_text = true;
	completion_popup_math = true;
	completion_popup_text = false;
	completion_popup_delay = 2.0;
	completion_popup_after_complete = true;
	autocorrection_math = false;
	completion_inline_math = true;
	completion_inline_text = false;
	completion_inline_dots = -1;
	completion_inline_delay = 0.2;
	default_decimal_point = ".";
	default_length_unit = Length::CM;
	cursor_width = 1;
	close_buffer_with_last_view = "yes";
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


bool LyXRC::read(FileName const & filename, bool check_format)
{
	Lexer lexrc(lyxrcTags);
	lexrc.setFile(filename);
	LYXERR(Debug::LYXRC, "Reading '" << filename << "'...");
	ReturnValues retval = read(lexrc, check_format);
	if (!check_format || retval != FormatMismatch)
		return retval == ReadOK;

	LYXERR(Debug::FILES, "Converting LyXRC file to " << LYXRC_FILEFORMAT);
	TempFile tmp("convert_lyxrc");
	FileName const tempfile = tmp.name();
	bool const success = prefs2prefs(filename, tempfile, false);
	if (!success) {
		LYXERR0 ("Unable to convert " << filename.absFileName() <<
			" to format " << LYXRC_FILEFORMAT);
		return false;
	} else {
		// Keep this in the else branch, such that lexrc2 goes out
		// of scope and releases the lock on tempfile before we
		// attempt to remove it. This matters on Windows.
		Lexer lexrc2(lyxrcTags);
		lexrc2.setFile(tempfile);
		LYXERR(Debug::LYXRC, "Reading '" << tempfile << "'...");
		retval = read(lexrc2, check_format);
	}
	return retval == ReadOK;
}


// don't need to worry about conversion, because this is always
// from an internal source
bool LyXRC::read(istream & is)
{
	Lexer lexrc(lyxrcTags);
	lexrc.setStream(is);
	LYXERR(Debug::LYXRC, "Reading istream...");
	return read(lexrc, false) == ReadOK;
}


LyXRC::ReturnValues LyXRC::read(Lexer & lexrc, bool check_format)
{
	if (lyxerr.debugging(Debug::PARSER))
		lexrc.printTable(lyxerr);

	if (!lexrc.isOK())
		return ReadError;

	// format prior to 2.0 and introduction of format tag
	unsigned int format = 0;

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
		default:
			break;
		}
		switch (static_cast<LyXRCTags>(le)) {
		case RC_LYXRCFORMAT:
			if (lexrc.next())
				format = lexrc.getInteger();
			break;
		case RC_INPUT: // Include file
			if (lexrc.next()) {
				FileName const tmp =
					libFileSearch(string(),
						      lexrc.getString());
				if (read(tmp, check_format)) {
					lexrc.printError(
					    "Error reading included file: " + tmp.absFileName());
				}
			}
			break;
		case RC_BINDFILE:
			if (lexrc.next())
				bind_file = os::internal_path(lexrc.getString());
			break;

		case RC_DEFFILE:
			if (lexrc.next())
				def_file = os::internal_path(lexrc.getString());
			break;

		case RC_UIFILE:
			if (lexrc.next())
				ui_file = os::internal_path(lexrc.getString());
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

		case RC_TEXINPUTS_PREFIX:
			lexrc >> texinputs_prefix;
			break;

		case RC_KBMAP:
			lexrc >> use_kbmap;
			break;

		case RC_KBMAP_PRIMARY:
			if (lexrc.next()) {
				string const kmap(os::internal_path(lexrc.getString()));
				if (!libFileSearch("kbd", kmap, "kmap").empty()
					  || kmap.empty()) {
					primary_kbmap = kmap;
				} else {
					lexrc.printError("LyX: Keymap `$$Token' not found");
				}
			}
			break;

		case RC_KBMAP_SECONDARY:
			if (lexrc.next()) {
				string const kmap(os::internal_path(lexrc.getString()));
				if (!libFileSearch("kbd", kmap, "kmap").empty()
					  || kmap.empty()) {
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

		case RC_BIBTEX_ALTERNATIVES:
			if (lexrc.next(true)) {
				bibtex_alternatives.insert(lexrc.getString());
			}
			break;

		case RC_BIBTEX_COMMAND:
			if (lexrc.next(true)) {
				bibtex_command = lexrc.getString();
			}
			break;

		case RC_JBIBTEX_COMMAND:
			if (lexrc.next(true)) {
				jbibtex_command = lexrc.getString();
			}
			break;

		case RC_INDEX_ALTERNATIVES:
			if (lexrc.next(true)) {
				index_alternatives.insert(lexrc.getString());
			}
			break;

		case RC_INDEX_COMMAND:
			if (lexrc.next(true)) {
				index_command = lexrc.getString();
			}
			break;

		case RC_JINDEX_COMMAND:
			if (lexrc.next(true)) {
				jindex_command = lexrc.getString();
			}
			break;

		case RC_SPLITINDEX_COMMAND:
			if (lexrc.next(true)) {
				splitindex_command = lexrc.getString();
			}
			break;

		case RC_NOMENCL_COMMAND:
			if (lexrc.next(true)) {
				nomencl_command = lexrc.getString();
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

		case RC_THESAURUSDIRPATH:
			if (lexrc.next()) {
				thesaurusdir_path = os::internal_path(lexrc.getString());
				thesaurusdir_path = expandPath(thesaurusdir_path);
			}
			break;

		case RC_HUNSPELLDIR_PATH:
			if (lexrc.next()) {
				hunspelldir_path = os::internal_path(lexrc.getString());
				hunspelldir_path = expandPath(hunspelldir_path);
			}
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

		case RC_AUTOCORRECTION_MATH:
			lexrc >> autocorrection_math;
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

		case RC_COMPLETION_MINLENGTH:
			lexrc >> completion_minlength;
			break;

		case RC_NUMLASTFILES:
			lexrc >> num_lastfiles;
			break;

		case RC_CHECKLASTFILES:
			lexrc >> check_lastfiles;
			break;

		case RC_ICON_SET:
			lexrc >> icon_set;
			break;

		case RC_USE_SYSTEM_THEME_ICONS:
			lexrc >> use_system_theme_icons;
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

		case RC_CURSOR_WIDTH:
			lexrc >> cursor_width;
			break;

		case RC_SCROLL_BELOW_DOCUMENT:
			lexrc >> scroll_below_document;
			break;

		case RC_PARAGRAPH_MARKERS:
			lexrc >> paragraph_markers;
			break;

		case RC_MAC_DONTSWAP_CTRL_META:
			lexrc >> mac_dontswap_ctrl_meta;
			break;

		case RC_MAC_LIKE_CURSOR_MOVEMENT:
			lexrc >> mac_like_cursor_movement;
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

		case RC_PLAINTEXT_LINELEN:
			lexrc >> plaintext_linelen;
			break;
			// Spellchecker settings:
		case RC_ACCEPT_COMPOUND:
			lexrc >> spellchecker_accept_compound;
			break;
		case RC_USE_SYSTEM_COLORS:
			lexrc >> use_system_colors;
			break;
		case RC_USE_TOOLTIP:
			lexrc >> use_tooltip;
			break;
		case RC_USE_PIXMAP_CACHE:
			lexrc >> use_pixmap_cache;
			break;
		case RC_USE_QIMAGE:
			lexrc >> use_qimage;
			break;
		case RC_SPELLCHECKER:
			lexrc >> spellchecker;
			break;
		case RC_ALT_LANG:
			lexrc >> spellchecker_alt_lang;
			break;
		case RC_ESC_CHARS:
			lexrc >> spellchecker_esc_chars;
			break;
		case RC_SPELLCHECK_CONTINUOUSLY:
			lexrc >> spellcheck_continuously;
			break;
		case RC_SPELLCHECK_NOTES:
			lexrc >> spellcheck_notes;
			break;
		case RC_MAKE_BACKUP:
			lexrc >> make_backup;
			break;
		case RC_SAVE_COMPRESSED:
			lexrc >> save_compressed;
			break;
		case RC_BACKUPDIR_PATH:
			if (lexrc.next()) {
				backupdir_path = os::internal_path(lexrc.getString());
				backupdir_path = expandPath(backupdir_path);
			}
			break;
		case RC_DEFAULT_DECIMAL_POINT:
			lexrc >> default_decimal_point;
			break;
		case RC_DEFAULT_LENGTH_UNIT:
			if (lexrc.next())
				default_length_unit = (Length::UNIT) lexrc.getInteger();
			break;
		case RC_DATE_INSERT_FORMAT:
			lexrc >> date_insert_format;
			break;
		case RC_LANGUAGE_CUSTOM_PACKAGE:
			lexrc >> language_custom_package;
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
		case RC_LANGUAGE_PACKAGE_SELECTION:
			if (lexrc.next()) {
				switch (lexrc.getInteger()) {
				case 0:
					language_package_selection = LP_AUTO;
					break;
				case 1:
					language_package_selection = LP_BABEL;
					break;
				case 2:
					language_package_selection = LP_CUSTOM;
					break;
				case 3:
					language_package_selection = LP_NONE;
					break;
				}
			}
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
		case RC_VISUAL_CURSOR:
			lexrc >> visual_cursor;
			break;
		case RC_CLOSE_BUFFER_WITH_LAST_VIEW:
			lexrc >> close_buffer_with_last_view;
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
		case RC_FILEFORMAT: {
			bool ok = true;
			string format, extensions, prettyname, shortcut;
			if (!(lexrc >> format >> extensions))
				ok = false;
			if (ok && lexrc.next(true))
				prettyname  = lexrc.getString();
			else
				ok = false;
			if (ok)
				if(!(lexrc >> shortcut))
					ok = false;
			string viewer, editor;
			if (ok && lexrc.next(true))
				viewer = lexrc.getString();
			else
				ok = false;
			if (ok && lexrc.next(true))
				editor = lexrc.getString();
			else
				ok = false;
			string flags, mime;
			if (!(lexrc >> flags >> mime))
				ok = false;
			int flgs = Format::none;
			while (!flags.empty()) {
				string flag;
				flags = split(flags, flag, ',');
				if (flag == "document")
					flgs |= Format::document;
				else if (flag == "vector")
					flgs |= Format::vector;
				else if (flag == "zipped=native")
					flgs |= Format::zipped_native;
				else if (flag == "menu=export")
					flgs |= Format::export_menu;
				else
					LYXERR0("Ignoring unknown flag `"
					       << flag << "' for format `"
					       << format << "'.");
			}
			if (!ok)
				LYXERR0("Syntax error in format " << format);
			else if (prettyname.empty()) {
				if (theConverters().formatIsUsed(format))
					LYXERR0("Can't delete format " << format);
				else
					formats.erase(format);
			} else {
				formats.add(format, extensions, prettyname,
					    shortcut, viewer, editor, mime, flgs);
			}
			break;
		}
		case RC_VIEWER_ALTERNATIVES:  {
			string format, command;
			lexrc >> format;
			lexrc >> command;
			viewer_alternatives[format].insert(command);
			break;
		}
		case RC_EDITOR_ALTERNATIVES:  {
			string format, command;
			lexrc >> format;
			lexrc >> command;
			editor_alternatives[format].insert(command);
			break;
		}

		case RC_DEFAULT_OTF_VIEW_FORMAT:
			lexrc >> default_otf_view_format;
			break;

		case RC_DEFAULT_VIEW_FORMAT:
			lexrc >> default_view_format;
			break;

		case RC_GUI_LANGUAGE:
			lexrc >> gui_language;
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
		case RC_FULL_SCREEN_STATUSBAR:
			lexrc >> full_screen_statusbar;
			break;
		case RC_FULL_SCREEN_TABBAR:
			lexrc >> full_screen_tabbar;
			break;
		case RC_FULL_SCREEN_MENUBAR:
			lexrc >> full_screen_menubar;
			break;
		case RC_FULL_SCREEN_WIDTH:
			lexrc >> full_screen_width;
			break;
		case RC_OPEN_BUFFERS_IN_TABS:
			lexrc >> open_buffers_in_tabs;
			break;
		case RC_SINGLE_CLOSE_TAB_BUTTON:
			lexrc >> single_close_tab_button;
			break;
		case RC_SINGLE_INSTANCE:
			lexrc >> single_instance;
			if (run_mode == PREFERRED)
				run_mode = single_instance ? USE_REMOTE : NEW_INSTANCE;
			break;
		case RC_FORWARD_SEARCH_DVI:
			if (lexrc.next(true))
				forward_search_dvi = lexrc.getString();
			break;
		case RC_FORWARD_SEARCH_PDF:
			if (lexrc.next(true))
				forward_search_pdf = lexrc.getString();
			break;
		case RC_EXPORT_OVERWRITE:
			if (lexrc.next()) {
				string const tmp = lexrc.getString();
				if (tmp == "all" || tmp == "true")
					export_overwrite = ALL_FILES;
				else if (tmp == "main")
					export_overwrite = MAIN_FILE;
				else {
					export_overwrite = NO_FILES;
					if (tmp != "ask" && tmp != "false")
						LYXERR0("Unrecognized export_overwrite status \""
						       << tmp << '"');
				}
			}
			break;
		case RC_SCROLL_WHEEL_ZOOM:
			if (lexrc.next()) {
				string const tmp = lexrc.getString();
				if (tmp == "ctrl")
					scroll_wheel_zoom = SCROLL_WHEEL_ZOOM_CTRL;
				else if (tmp == "shift")
					scroll_wheel_zoom = SCROLL_WHEEL_ZOOM_SHIFT;
				else if (tmp == "alt")
					scroll_wheel_zoom = SCROLL_WHEEL_ZOOM_ALT;
				else {
					scroll_wheel_zoom = SCROLL_WHEEL_ZOOM_OFF;
					if (tmp != "off" && tmp != "false")
						LYXERR0("Unrecognized scroll_wheel_zoom status \""
						       << tmp << '"');
				}
			}
			break;

		case RC_LAST:
			break; // this is just a dummy
		}

		// This is triggered the first time through the loop unless
		// we hit a format tag.
		if (check_format && format != LYXRC_FILEFORMAT)
			return FormatMismatch;
	}

	/// Update converters data-structures
	theConverters().update(formats);
	theConverters().buildGraph();

	return ReadOK;
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
		os << "# LyX " << lyx_version
		   << " generated this file. If you want to make your own\n"
		   << "# modifications you should do them from inside LyX and save.\n\n"
		   << "Format " << LYXRC_FILEFORMAT << "\n\n";

	// Why the switch you might ask. It is a trick to ensure that all
	// the elements in the LyXRCTags enum are handled. As you can see
	// there are no breaks at all. So it is just a huge fall-through.
	// The nice thing is that we will get a warning from the compiler
	// if we forget an element.
	switch (tag) {
	case RC_LAST:
	case RC_INPUT:
		// input/include files are not done here
	case RC_LYXRCFORMAT:
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
	case RC_CHKTEX_COMMAND:
		if (ignore_system_lyxrc ||
		    chktex_command != system_lyxrc.chktex_command) {
			os << "\\chktex_command \"" << escapeCommand(chktex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_BIBTEX_ALTERNATIVES: {
		CommandSet::const_iterator it = bibtex_alternatives.begin();
		CommandSet::const_iterator end = bibtex_alternatives.end();
		for ( ; it != end; ++it) {
			if (ignore_system_lyxrc
			    || !system_lyxrc.bibtex_alternatives.count(*it))
				os << "\\bibtex_alternatives \""
				   << *it << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	}
	case RC_BIBTEX_COMMAND:
		if (ignore_system_lyxrc ||
		    bibtex_command != system_lyxrc.bibtex_command) {
			os << "\\bibtex_command \"" << escapeCommand(bibtex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_JBIBTEX_COMMAND:
		if (ignore_system_lyxrc ||
		    jbibtex_command != system_lyxrc.jbibtex_command) {
			os << "\\jbibtex_command \"" << escapeCommand(jbibtex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_INDEX_ALTERNATIVES: {
		CommandSet::const_iterator it = index_alternatives.begin();
		CommandSet::const_iterator end = index_alternatives.end();
		for ( ; it != end; ++it) {
			if (ignore_system_lyxrc
			    || !system_lyxrc.index_alternatives.count(*it))
				os << "\\index_alternatives \""
				   << *it << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	}
	case RC_INDEX_COMMAND:
		if (ignore_system_lyxrc ||
		    index_command != system_lyxrc.index_command) {
			os << "\\index_command \"" << escapeCommand(index_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_JINDEX_COMMAND:
		if (ignore_system_lyxrc ||
		    jindex_command != system_lyxrc.jindex_command) {
			os << "\\jindex_command \"" << escapeCommand(jindex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_SPLITINDEX_COMMAND:
		if (ignore_system_lyxrc ||
		    splitindex_command != system_lyxrc.splitindex_command) {
			os << "\\splitindex_command \"" << escapeCommand(splitindex_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_NOMENCL_COMMAND:
		if (ignore_system_lyxrc ||
		    nomencl_command != system_lyxrc.nomencl_command) {
			os << "\\nomencl_command \"" << escapeCommand(nomencl_command) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_TEX_EXPECTS_WINDOWS_PATHS:
		// Don't write this setting to the preferences file,
		// but allow temporary changes (bug 7557).
		if (ignore_system_lyxrc) {
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

	case RC_ICON_SET:
		if (ignore_system_lyxrc ||
		    icon_set != system_lyxrc.icon_set) {
			os << "\\icon_set \"" << icon_set
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;

	case RC_USE_SYSTEM_THEME_ICONS:
		if (ignore_system_lyxrc ||
			  use_system_theme_icons != system_lyxrc.use_system_theme_icons) {
			os << "\\use_system_theme_icons "
				<< convert<string>(use_system_theme_icons)
				<< "\n";
		}
		if (tag != RC_LAST)
			break;

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
	case RC_CURSOR_WIDTH:
		if (ignore_system_lyxrc ||
			cursor_width
			!= system_lyxrc.cursor_width) {
			os << "\\cursor_width "
			<< cursor_width << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SCROLL_BELOW_DOCUMENT:
		if (ignore_system_lyxrc ||
		    scroll_below_document
		    != system_lyxrc.scroll_below_document) {
			os << "\\scroll_below_document "
			   << convert<string>(scroll_below_document) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_PARAGRAPH_MARKERS:
		if (ignore_system_lyxrc ||
			paragraph_markers
		    != system_lyxrc.paragraph_markers) {
			os << "\\paragraph_markers "
			   << convert<string>(paragraph_markers) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_MAC_DONTSWAP_CTRL_META:
		if (ignore_system_lyxrc ||
		    mac_dontswap_ctrl_meta
		    != system_lyxrc.mac_dontswap_ctrl_meta) {
			os << "\\mac_dontswap_ctrl_meta "
			   << convert<string>(mac_dontswap_ctrl_meta) << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_MAC_LIKE_CURSOR_MOVEMENT:
		if (ignore_system_lyxrc ||
		    mac_like_cursor_movement
		    != system_lyxrc.mac_like_cursor_movement) {
			os << "\\mac_like_cursor_movement "
			   << convert<string>(mac_like_cursor_movement) << '\n';
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
	case RC_FULL_SCREEN_STATUSBAR:
		if (ignore_system_lyxrc ||
		    full_screen_statusbar != system_lyxrc.full_screen_statusbar) {
			os << "\\fullscreen_statusbar "
			   << convert<string>(full_screen_statusbar)
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
	case RC_FULL_SCREEN_MENUBAR:
		if (ignore_system_lyxrc ||
		    full_screen_menubar != system_lyxrc.full_screen_menubar) {
			os << "\\fullscreen_menubar "
			   << convert<string>(full_screen_menubar)
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
	case RC_SINGLE_CLOSE_TAB_BUTTON:
		if (ignore_system_lyxrc ||
		    single_close_tab_button != system_lyxrc.single_close_tab_button) {
			os << "\\single_close_tab_button "
			   << convert<string>(single_close_tab_button)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_SINGLE_INSTANCE:
		if (ignore_system_lyxrc ||
		    single_instance != system_lyxrc.single_instance) {
			os << "\\single_instance "
			   << convert<string>(single_instance)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_FORWARD_SEARCH_DVI:
		if (ignore_system_lyxrc ||
		    forward_search_dvi != system_lyxrc.forward_search_dvi) {
			os << "\\forward_search_dvi \"" << escapeCommand(forward_search_dvi) << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_FORWARD_SEARCH_PDF:
		if (ignore_system_lyxrc ||
		    forward_search_pdf != system_lyxrc.forward_search_pdf) {
			os << "\\forward_search_pdf \"" << escapeCommand(forward_search_pdf) << "\"\n";
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
		   << "# TEX SECTION #######################################\n"
		   << "#\n\n";

	case RC_TEXINPUTS_PREFIX:
		if (ignore_system_lyxrc ||
		    texinputs_prefix != system_lyxrc.texinputs_prefix) {
			os << "\\texinputs_prefix \"" << texinputs_prefix << "\"\n";
		}
		if (tag != RC_LAST)
			break;

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
	case RC_AUTOCORRECTION_MATH:
		if (ignore_system_lyxrc ||
		    autocorrection_math != system_lyxrc.autocorrection_math) {
			os << "\\autocorrection_math "
				<< convert<string>(autocorrection_math) << '\n';
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
	case RC_COMPLETION_MINLENGTH:
		if (ignore_system_lyxrc ||
			completion_minlength != system_lyxrc.completion_minlength) {
			os << "\\completion_minlength " << convert<string>(completion_minlength)
			<< '\n';
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
	case RC_THESAURUSDIRPATH:
		if (ignore_system_lyxrc ||
		    thesaurusdir_path != system_lyxrc.thesaurusdir_path) {
			string const path = os::external_path(thesaurusdir_path);
			os << "\\thesaurusdir_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_HUNSPELLDIR_PATH:
		if (ignore_system_lyxrc ||
		    hunspelldir_path != system_lyxrc.hunspelldir_path) {
			string const path = os::external_path(hunspelldir_path);
			os << "\\hunspelldir_path \"" << path << "\"\n";
		}
		if (tag != RC_LAST)
			break;
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
	case RC_SAVE_COMPRESSED:
		if (ignore_system_lyxrc ||
		    save_compressed != system_lyxrc.save_compressed) {
			os << "\\save_compressed " << convert<string>(save_compressed) << '\n';
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

		os << "\n#\n"
		   << "# SPELLCHECKER SECTION ##############################\n"
		   << "#\n\n";

	case RC_ACCEPT_COMPOUND:
		if (ignore_system_lyxrc ||
		    spellchecker_accept_compound != system_lyxrc.spellchecker_accept_compound) {
			os << "\\accept_compound " << convert<string>(spellchecker_accept_compound)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_ALT_LANG:
		if (ignore_system_lyxrc ||
		    spellchecker_alt_lang != system_lyxrc.spellchecker_alt_lang) {
			os << "\\alternate_language \"" << spellchecker_alt_lang
			   << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_ESC_CHARS:
		if (ignore_system_lyxrc ||
		    spellchecker_esc_chars != system_lyxrc.spellchecker_esc_chars) {
			os << "\\escape_chars \"" << spellchecker_esc_chars << "\"\n";
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_SYSTEM_COLORS:
		if (ignore_system_lyxrc ||
		    use_system_colors != system_lyxrc.use_system_colors) {
			os << "\\use_system_colors "
			   << convert<string>(use_system_colors)
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
		if (tag != RC_LAST)
			break;
	case RC_USE_PIXMAP_CACHE:
		if (ignore_system_lyxrc ||
		    use_pixmap_cache != system_lyxrc.use_pixmap_cache) {
			os << "\\use_pixmap_cache "
			   << convert<string>(use_pixmap_cache)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_USE_QIMAGE:
		if (ignore_system_lyxrc ||
		    use_qimage != system_lyxrc.use_qimage) {
			os << "\\use_qimage "
			   << convert<string>(use_qimage)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# LANGUAGE SUPPORT SECTION ##########################\n"
		   << "#\n\n";
		if (tag != RC_LAST)
			break;

	case RC_DEFAULT_DECIMAL_POINT:
		if (ignore_system_lyxrc ||
		    default_decimal_point != system_lyxrc.default_decimal_point) {
			os << "\\default_decimal_point " << default_decimal_point << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_DEFAULT_LENGTH_UNIT:
		if (ignore_system_lyxrc ||
		    default_length_unit != system_lyxrc.default_length_unit) {
			os << "\\default_length_unit " << int(default_length_unit) << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_SPELLCHECKER:
		if (ignore_system_lyxrc ||
		    spellchecker != system_lyxrc.spellchecker) {
			os << "\\spellchecker " << spellchecker << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_SPELLCHECK_CONTINUOUSLY:
		if (ignore_system_lyxrc ||
		    spellcheck_continuously != system_lyxrc.spellcheck_continuously) {
			os << "\\spellcheck_continuously " << convert<string>(spellcheck_continuously)
			   << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_SPELLCHECK_NOTES:
		if (ignore_system_lyxrc ||
		    spellcheck_notes != system_lyxrc.spellcheck_notes) {
			os << "\\spellcheck_notes " << convert<string>(spellcheck_notes)
			   << '\n';
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
	case RC_CLOSE_BUFFER_WITH_LAST_VIEW:
		if (ignore_system_lyxrc ||
			close_buffer_with_last_view != system_lyxrc.close_buffer_with_last_view) {
			os << "# When closing last view, buffer closes (yes), hides (no), or ask the user (ask)\n";
			os << "\\close_buffer_with_last_view " << close_buffer_with_last_view << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_LANGUAGE_CUSTOM_PACKAGE:
		if (ignore_system_lyxrc ||
		    language_custom_package != system_lyxrc.language_custom_package) {
			os << "\\language_custom_package \"" << language_custom_package
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
	case RC_LANGUAGE_PACKAGE_SELECTION:
		if (ignore_system_lyxrc ||
		    language_package_selection != system_lyxrc.language_package_selection) {
			os << "\\language_package_selection ";
			switch (language_package_selection) {
			case LP_AUTO:
				os << "0\n";
				break;
			case LP_BABEL:
				os << "1\n";
				break;
			case LP_CUSTOM:
				os << "2\n";
				break;
			case LP_NONE:
				os << "3\n";
				break;
			}
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
	case RC_GUI_LANGUAGE:
		if (ignore_system_lyxrc ||
		    gui_language != system_lyxrc.gui_language) {
			os << "\\gui_language " << gui_language << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_EXPORT_OVERWRITE:
		if (ignore_system_lyxrc ||
		    export_overwrite != system_lyxrc.export_overwrite) {
			string status;
			switch (export_overwrite) {
			case NO_FILES:
				status = "ask";
				break;
			case MAIN_FILE:
				status = "main";
				break;
			case ALL_FILES:
				status = "all";
				break;
			}
			os << "\\export_overwrite " << status << '\n';
		}
		if (tag != RC_LAST)
			break;

	case RC_SCROLL_WHEEL_ZOOM:
		if (ignore_system_lyxrc ||
			scroll_wheel_zoom != system_lyxrc.scroll_wheel_zoom) {
			string status;
			switch (scroll_wheel_zoom) {
			case SCROLL_WHEEL_ZOOM_OFF:
				status = "off";
				break;
			case SCROLL_WHEEL_ZOOM_CTRL:
				status = "ctrl";
				break;
			case SCROLL_WHEEL_ZOOM_SHIFT:
				status = "shift";
				break;
			case SCROLL_WHEEL_ZOOM_ALT:
				status = "alt";
				break;
			}
			os << "\\scroll_wheel_zoom " << status << '\n';
		}
		if (tag != RC_LAST)
			break;

		os << "\n#\n"
		   << "# FORMATS SECTION ##########################\n"
		   << "#\n\n";

	case RC_FILEFORMAT:
		// New/modified formats
		for (Formats::const_iterator cit = formats.begin();
		     cit != formats.end(); ++cit) {
			Format const * format =
				system_formats.getFormat(cit->name());
			if (!format ||
			    format->extensions() != cit->extensions() ||
			    format->prettyname() != cit->prettyname() ||
			    format->shortcut() != cit->shortcut() ||
			    format->viewer() != cit->viewer() ||
			    format->editor() != cit->editor() ||
			    format->documentFormat() != cit->documentFormat() ||
			    format->vectorFormat() != cit->vectorFormat() ||
			    format->inExportMenu() != cit->inExportMenu() ||
			    format->mime() != cit->mime()) {
				os << "\\format \"" << cit->name() << "\" \""
				   << cit->extensions() << "\" \""
				   << cit->prettyname() << "\" \""
				   << cit->shortcut() << "\" \""
				   << escapeCommand(cit->viewer()) << "\" \""
				   << escapeCommand(cit->editor()) << "\" \"";
				vector<string> flags;
				if (cit->documentFormat())
					flags.push_back("document");
				if (cit->vectorFormat())
					flags.push_back("vector");
				if (cit->zippedNative())
					flags.push_back("zipped=native");
				if (cit->inExportMenu())
					flags.push_back("menu=export");

				os << getStringFromVector(flags);
				os << "\" \"" << cit->mime() << "\"\n";
			}
		}

		// Look for deleted formats
		for (Formats::const_iterator cit = system_formats.begin();
		     cit != system_formats.end(); ++cit)
			if (!formats.getFormat(cit->name()))
				os << "\\format \"" << cit->name()
				   << "\" \"\" \"\" \"\" \"\" \"\" \"\" \"\"\n";
		if (tag != RC_LAST)
			break;
	case RC_VIEWER_ALTERNATIVES: {
		Alternatives::const_iterator it = viewer_alternatives.begin();
		Alternatives::const_iterator const en = viewer_alternatives.end();
		Alternatives::const_iterator const sysend =
				system_lyxrc.viewer_alternatives.end();
 		for (; it != en; ++it) {
			string const & fmt = it->first;
			CommandSet const & cmd = it->second;
			CommandSet::const_iterator sit = cmd.begin();
			CommandSet::const_iterator const sen = cmd.end();
			Alternatives::const_iterator const sysfmt = ignore_system_lyxrc ?
					system_lyxrc.viewer_alternatives.begin() : // we won't use it in this case
					system_lyxrc.viewer_alternatives.find(fmt);
			for (; sit != sen; ++sit) {
				string const & cmd = *sit;
				if (ignore_system_lyxrc
				    || sysfmt == sysend               // format not found
					 || sysfmt->second.count(cmd) == 0 // this command not found
				   )
					os << "\\viewer_alternatives " << fmt << " \"" << escapeCommand(cmd) << "\"\n";
			}
		}
		if (tag != RC_LAST)
			break;
	}
	case RC_EDITOR_ALTERNATIVES: {
		Alternatives::const_iterator it = editor_alternatives.begin();
		Alternatives::const_iterator const en = editor_alternatives.end();
		Alternatives::const_iterator const sysend =
				system_lyxrc.editor_alternatives.end();
		for (; it != en; ++it) {
			string const & fmt = it->first;
			CommandSet const & cmd = it->second;
			CommandSet::const_iterator sit = cmd.begin();
			CommandSet::const_iterator const sen = cmd.end();
			Alternatives::const_iterator const sysfmt = ignore_system_lyxrc ?
					system_lyxrc.editor_alternatives.begin() : // we won't use it in this case
					system_lyxrc.editor_alternatives.find(fmt);
			for (; sit != sen; ++sit) {
				string const & cmd = *sit;
				if (ignore_system_lyxrc
				    || sysfmt == sysend               // format not found
				    || sysfmt->second.count(cmd) == 0 // this command not found
				   )
					os << "\\editor_alternatives " << fmt << " \"" << escapeCommand(cmd) << "\"\n";
			}
		}
		if (tag != RC_LAST)
			break;
	}
	case RC_DEFAULT_OTF_VIEW_FORMAT:
        if ((ignore_system_lyxrc ||
            default_otf_view_format != system_lyxrc.default_otf_view_format)
            && !default_otf_view_format.empty()) {
			os << "\\default_otf_view_format " << default_otf_view_format << '\n';
		}
		if (tag != RC_LAST)
			break;
	case RC_DEFAULT_VIEW_FORMAT:
		if (ignore_system_lyxrc ||
		    default_view_format != system_lyxrc.default_view_format) {
			os << "\\default_view_format " << default_view_format << '\n';
		}
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
				theSystemConverters().getConverter(cit->from(),
				                                   cit->to());
			if (!converter ||
			    converter->command() != cit->command() ||
			    converter->flags() != cit->flags())
				os << "\\converter \"" << cit->from() << "\" \""
				   << cit->to() << "\" \""
				   << escapeCommand(cit->command()) << "\" \""
				   << cit->flags() << "\"\n";
		}

		// New/modifed converters
		for (Converters::const_iterator cit = theSystemConverters().begin();
		     cit != theSystemConverters().end(); ++cit)
			if (!theConverters().getConverter(cit->from(), cit->to()))
				os << "\\converter \"" << cit->from()
				   << "\" \"" << cit->to() << "\" \"\" \"\"\n";
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


void actOnUpdatedPrefs(LyXRC const & lyxrc_orig, LyXRC const & lyxrc_new)
{
	// Why the switch you might ask. It is a trick to ensure that all
	// the elements in the LyXRCTags enum is handled. As you can see
	// there are no breaks at all. So it is just a huge fall-through.
	// The nice thing is that we will get a warning from the compiler
	// if we forget an element.
	LyXRC::LyXRCTags tag = LyXRC::RC_LAST;
	switch (tag) {
	case LyXRC::RC_LAST:
	case LyXRC::RC_ACCEPT_COMPOUND:
		if (lyxrc_orig.spellchecker_accept_compound != lyxrc_new.spellchecker_accept_compound)
			if (theSpellChecker()) theSpellChecker()->advanceChangeNumber();
	case LyXRC::RC_ALT_LANG:
	case LyXRC::RC_PLAINTEXT_LINELEN:
	case LyXRC::RC_AUTOCORRECTION_MATH:
	case LyXRC::RC_AUTOREGIONDELETE:
	case LyXRC::RC_AUTORESET_OPTIONS:
	case LyXRC::RC_AUTOSAVE:
	case LyXRC::RC_AUTO_NUMBER:
	case LyXRC::RC_BACKUPDIR_PATH:
	case LyXRC::RC_BIBTEX_ALTERNATIVES:
	case LyXRC::RC_BIBTEX_COMMAND:
	case LyXRC::RC_BINDFILE:
	case LyXRC::RC_CHECKLASTFILES:
	case LyXRC::RC_COMPLETION_CURSOR_TEXT:
	case LyXRC::RC_COMPLETION_INLINE_DELAY:
	case LyXRC::RC_COMPLETION_INLINE_DOTS:
	case LyXRC::RC_COMPLETION_INLINE_MATH:
	case LyXRC::RC_COMPLETION_INLINE_TEXT:
	case LyXRC::RC_COMPLETION_POPUP_AFTER_COMPLETE:
	case LyXRC::RC_COMPLETION_POPUP_DELAY:
	case LyXRC::RC_COMPLETION_POPUP_MATH:
	case LyXRC::RC_COMPLETION_POPUP_TEXT:
	case LyXRC::RC_COMPLETION_MINLENGTH:
	case LyXRC::RC_USELASTFILEPOS:
	case LyXRC::RC_LOADSESSION:
	case LyXRC::RC_CHKTEX_COMMAND:
	case LyXRC::RC_CONVERTER:
	case LyXRC::RC_CONVERTER_CACHE_MAXAGE:
	case LyXRC::RC_COPIER:
	case LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR:
	case LyXRC::RC_SCROLL_BELOW_DOCUMENT:
	case LyXRC::RC_DATE_INSERT_FORMAT:
	case LyXRC::RC_GUI_LANGUAGE:
	case LyXRC::RC_DEFAULT_OTF_VIEW_FORMAT:
	case LyXRC::RC_DEFAULT_VIEW_FORMAT:
	case LyXRC::RC_DEFFILE:
	case LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN:
	case LyXRC::RC_DISPLAY_GRAPHICS:
	case LyXRC::RC_DOCUMENTPATH:
		if (lyxrc_orig.document_path != lyxrc_new.document_path) {
			FileName path(lyxrc_new.document_path);
			if (path.exists() && path.isDirectory())
				package().document_dir() = FileName(lyxrc.document_path);
		}
	case LyXRC::RC_EDITOR_ALTERNATIVES:
	case LyXRC::RC_ESC_CHARS:
	case LyXRC::RC_EXAMPLEPATH:
	case LyXRC::RC_FONT_ENCODING:
	case LyXRC::RC_FILEFORMAT:
	case LyXRC::RC_GROUP_LAYOUTS:
	case LyXRC::RC_HUNSPELLDIR_PATH:
	case LyXRC::RC_ICON_SET:
	case LyXRC::RC_INDEX_ALTERNATIVES:
	case LyXRC::RC_INDEX_COMMAND:
	case LyXRC::RC_JBIBTEX_COMMAND:
	case LyXRC::RC_JINDEX_COMMAND:
	case LyXRC::RC_NOMENCL_COMMAND:
	case LyXRC::RC_INPUT:
	case LyXRC::RC_KBMAP:
	case LyXRC::RC_KBMAP_PRIMARY:
	case LyXRC::RC_KBMAP_SECONDARY:
	case LyXRC::RC_LANGUAGE_AUTO_BEGIN:
	case LyXRC::RC_LANGUAGE_AUTO_END:
	case LyXRC::RC_LANGUAGE_COMMAND_BEGIN:
	case LyXRC::RC_LANGUAGE_COMMAND_END:
	case LyXRC::RC_LANGUAGE_COMMAND_LOCAL:
	case LyXRC::RC_LANGUAGE_GLOBAL_OPTIONS:
	case LyXRC::RC_LANGUAGE_CUSTOM_PACKAGE:
	case LyXRC::RC_LANGUAGE_PACKAGE_SELECTION:
	case LyXRC::RC_LYXRCFORMAT:
	case LyXRC::RC_MAC_DONTSWAP_CTRL_META:
	case LyXRC::RC_MAC_LIKE_CURSOR_MOVEMENT:
	case LyXRC::RC_MACRO_EDIT_STYLE:
	case LyXRC::RC_MAKE_BACKUP:
	case LyXRC::RC_MARK_FOREIGN_LANGUAGE:
	case LyXRC::RC_MOUSE_WHEEL_SPEED:
	case LyXRC::RC_NUMLASTFILES:
	case LyXRC::RC_PARAGRAPH_MARKERS:
	case LyXRC::RC_PATH_PREFIX:
		if (lyxrc_orig.path_prefix != lyxrc_new.path_prefix) {
			prependEnvPath("PATH", lyxrc_new.path_prefix);
			// Resets python path
			support::os::python(true);
		}
	case LyXRC::RC_PREVIEW:
	case LyXRC::RC_PREVIEW_HASHED_LABELS:
	case LyXRC::RC_PREVIEW_SCALE_FACTOR:
	case LyXRC::RC_PRINTCOLLCOPIESFLAG:
	case LyXRC::RC_PRINTCOPIESFLAG:
	case LyXRC::RC_PRINTER:
	case LyXRC::RC_PRINTEVENPAGEFLAG:
	case LyXRC::RC_PRINTEXSTRAOPTIONS:
	case LyXRC::RC_PRINTFILEEXTENSION:
	case LyXRC::RC_PRINTLANDSCAPEFLAG:
	case LyXRC::RC_PRINTODDPAGEFLAG:
	case LyXRC::RC_PRINTPAGERANGEFLAG:
	case LyXRC::RC_PRINTPAPERDIMENSIONFLAG:
	case LyXRC::RC_PRINTPAPERFLAG:
	case LyXRC::RC_PRINTREVERSEFLAG:
	case LyXRC::RC_PRINTSPOOL_COMMAND:
	case LyXRC::RC_PRINTSPOOL_PRINTERPREFIX:
	case LyXRC::RC_PRINTTOFILE:
	case LyXRC::RC_PRINTTOPRINTER:
	case LyXRC::RC_PRINT_ADAPTOUTPUT:
	case LyXRC::RC_PRINT_COMMAND:
	case LyXRC::RC_SAVE_COMPRESSED:
	case LyXRC::RC_SCREEN_DPI:
	case LyXRC::RC_SCREEN_FONT_ROMAN:
	case LyXRC::RC_SCREEN_FONT_ROMAN_FOUNDRY:
	case LyXRC::RC_SCREEN_FONT_SANS:
	case LyXRC::RC_SCREEN_FONT_SANS_FOUNDRY:
	case LyXRC::RC_SCREEN_FONT_SCALABLE:
	case LyXRC::RC_SCREEN_FONT_SIZES:
	case LyXRC::RC_SCREEN_FONT_TYPEWRITER:
	case LyXRC::RC_SCREEN_FONT_TYPEWRITER_FOUNDRY:
	case LyXRC::RC_GEOMETRY_SESSION:
	case LyXRC::RC_SCREEN_ZOOM:
	case LyXRC::RC_SERVERPIPE:
	case LyXRC::RC_SET_COLOR:
	case LyXRC::RC_SHOW_BANNER:
	case LyXRC::RC_OPEN_BUFFERS_IN_TABS:
	case LyXRC::RC_SPELLCHECKER:
		if (lyxrc_orig.spellchecker != lyxrc_new.spellchecker)
			setSpellChecker();
	case LyXRC::RC_SPELLCHECK_CONTINUOUSLY:
	case LyXRC::RC_SPELLCHECK_NOTES:
	case LyXRC::RC_SPLITINDEX_COMMAND:
	case LyXRC::RC_TEMPDIRPATH:
	case LyXRC::RC_TEMPLATEPATH:
	case LyXRC::RC_TEX_ALLOWS_SPACES:
	case LyXRC::RC_TEX_EXPECTS_WINDOWS_PATHS:
		if (lyxrc_orig.windows_style_tex_paths != lyxrc_new.windows_style_tex_paths) {
			os::windows_style_tex_paths(lyxrc_new.windows_style_tex_paths);
		}
	case LyXRC::RC_TEXINPUTS_PREFIX:
	case LyXRC::RC_THESAURUSDIRPATH:
	case LyXRC::RC_UIFILE:
	case LyXRC::RC_USER_EMAIL:
	case LyXRC::RC_USER_NAME:
	case LyXRC::RC_USE_CONVERTER_CACHE:
	case LyXRC::RC_USE_SYSTEM_COLORS:
	case LyXRC::RC_USE_TOOLTIP:
	case LyXRC::RC_USE_PIXMAP_CACHE:
	case LyXRC::RC_USE_QIMAGE:
	case LyXRC::RC_USE_SYSTEM_THEME_ICONS:
	case LyXRC::RC_VIEWDVI_PAPEROPTION:
	case LyXRC::RC_SINGLE_CLOSE_TAB_BUTTON:
	case LyXRC::RC_SINGLE_INSTANCE:
	case LyXRC::RC_SORT_LAYOUTS:
	case LyXRC::RC_FULL_SCREEN_LIMIT:
	case LyXRC::RC_FULL_SCREEN_SCROLLBAR:
	case LyXRC::RC_FULL_SCREEN_MENUBAR:
	case LyXRC::RC_FULL_SCREEN_STATUSBAR:
	case LyXRC::RC_FULL_SCREEN_TABBAR:
	case LyXRC::RC_FULL_SCREEN_TOOLBARS:
	case LyXRC::RC_FULL_SCREEN_WIDTH:
	case LyXRC::RC_VISUAL_CURSOR:
	case LyXRC::RC_CLOSE_BUFFER_WITH_LAST_VIEW:
	case LyXRC::RC_VIEWER:
	case LyXRC::RC_VIEWER_ALTERNATIVES:
	case LyXRC::RC_FORWARD_SEARCH_DVI:
	case LyXRC::RC_FORWARD_SEARCH_PDF:
	case LyXRC::RC_EXPORT_OVERWRITE:
	case LyXRC::RC_DEFAULT_DECIMAL_POINT:
	case LyXRC::RC_DEFAULT_LENGTH_UNIT:
	case LyXRC::RC_SCROLL_WHEEL_ZOOM:
	case LyXRC::RC_CURSOR_WIDTH:
		break;
	}
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
		str = _("Specify an alternate language. The default is to use the language of the document.");
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

	case RC_JBIBTEX_COMMAND:
		str = _("Define the options of the bibtex program for PLaTeX (Japanese LaTeX).");
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

	case RC_CURSOR_WIDTH:
		str = _("Configure the width of the text cursor. Automatic zoom-controlled cursor width used when set to 0.");
		break;

	case RC_SCROLL_BELOW_DOCUMENT:
		str = _("LyX normally doesn't allow the user to scroll further than the bottom of the document. Set to true if you prefer to scroll the bottom of the document to the top of the screen");
		break;

	case RC_MAC_DONTSWAP_CTRL_META:
		str = _("Make Apple key act as Meta and Control key as Ctrl.");
		break;

	case RC_MAC_LIKE_CURSOR_MOVEMENT:
		str = _("Use the Mac OS X conventions for cursor movement");
		break;

	case RC_SHOW_MACRO_LABEL:
		str = _("Show a small box around a Math Macro with the macro name when the cursor is inside.");
		break;

	case RC_DATE_INSERT_FORMAT:
		//xgettext:no-c-format
		str = _("This accepts the normal strftime formats; see man strftime for full details. E.g.\"%A, %e. %B %Y\".");
		break;

	case RC_DEFFILE:
		str = _("Command definition file. Can either specify an absolute path, or LyX will look in its global and local commands/ directories.");
		break;

	case RC_DEFAULT_OTF_VIEW_FORMAT:
		str = _("The default format used with LFUN_BUFFER_[VIEW|UPDATE] with non-TeX fonts.");
		break;

	case RC_DEFAULT_VIEW_FORMAT:
		str = _("The default format used with LFUN_BUFFER_[VIEW|UPDATE].");
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
		str = _("Specify additional chars that can be part of a word.");
		break;

	case RC_EXAMPLEPATH:
		str = _("The path that LyX will set when offering to choose an example. An empty value selects the directory LyX was started from.");
		break;

	case RC_FONT_ENCODING:
		str = _("The font encoding used for the LaTeX2e fontenc package. T1 is highly recommended for non-English languages.");
		break;

	case RC_FILEFORMAT:
		break;

	case RC_INDEX_COMMAND:
		str = _("Define the options of makeindex (cf. man makeindex) or select an alternative compiler. E.g., using xindy/make-rules, the command string would be \"makeindex.sh -m $$lang\".");
		break;

	case RC_JINDEX_COMMAND:
		str = _("Define the options of the index program for PLaTeX (Japanese LaTeX).");
		break;

	case RC_NOMENCL_COMMAND:
		str = _("Define the options of makeindex (cf. man makeindex) to be used for nomenclatures. This might differ from the index processing options.");
		break;

	case RC_INPUT:
		break;

	case RC_KBMAP:
	case RC_KBMAP_PRIMARY:
	case RC_KBMAP_SECONDARY:
		str = _("Use this to set the correct mapping file for your keyboard. You'll need this if you for instance want to type German documents on an American keyboard.");
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

	case RC_LANGUAGE_CUSTOM_PACKAGE:
		str = _("The LaTeX command for loading the language package. E.g. \"\\usepackage{babel}\", \"\\usepackage{omega}\".");
		break;

	case RC_LANGUAGE_PACKAGE_SELECTION:
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

	case RC_AUTOCORRECTION_MATH:
		str = _("Allow TeXMacs shorthand, like => converting to \Rightarrow.");
		break;

	case RC_NUMLASTFILES:
		str = bformat(_("Maximal number of lastfiles. Up to %1$d can appear in the file menu."),
			maxlastfiles);
		break;

	case RC_PATH_PREFIX:
		str = _("Specify those directories which should be "
			 "prepended to the PATH environment variable.\n"
			 "Use the OS native format.");
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

	case RC_VISUAL_CURSOR:
		str = _("Select to have visual bidi cursor movement, unselect for logical movement.");
		break;

	case RC_CLOSE_BUFFER_WITH_LAST_VIEW:
		str = _("Specify whether, closing the last view of an open document, LyX should close the document (yes), hide it (no), or ask the user (ask).");
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

	case RC_TEMPDIRPATH:
		str = _("LyX will place its temporary directories in this path. They will be deleted when you quit LyX.");
		break;

	case RC_THESAURUSDIRPATH:
		str = _("This is the place where the files of the thesaurus library reside.");
		break;

	case RC_TEMPLATEPATH:
		str = _("The path that LyX will set when offering to choose a template. An empty value selects the directory LyX was started from.");
		break;

	case RC_TEX_ALLOWS_SPACES:
		break;

	case RC_TEX_EXPECTS_WINDOWS_PATHS:
		break;

	case RC_TEXINPUTS_PREFIX:
		str = _("Specify those directories which should be "
			 "prepended to the TEXINPUTS environment variable.\n"
			 "A '.' represents the current document directory. "
			 "Use the OS native format.");
		break;

	case RC_UIFILE:
		str = _("The UI (user interface) file. Can either specify an absolute path, or LyX will look in its global and local ui/ directories.");
		break;

	case RC_USER_EMAIL:
		break;

	case RC_USER_NAME:
		break;

	case RC_USE_USE_SYSTEM_COLORS:
		str = _("Enable use the system colors for some things like main window background and selection.");
		break;

	case RC_USE_TOOLTIP:
		str = _("Enable the automatic appearance of tool tips in the work area.");
		break;

	case RC_USE_PIXMAP_CACHE:
		str = _("Enable the pixmap cache that might improve performance on Mac and Windows.");
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
