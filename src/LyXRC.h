// -*- C++ -*-
/**
 * \file LyXRC.h
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

#ifndef LYXRC_H
#define LYXRC_H

#include "LyX.h"

#include "support/Length.h"
#include "support/strfwd.h"
#include "support/userinfo.h"

#include <map>
#include <set>
#include <string>


namespace lyx {

namespace support { class FileName; }

class Lexer;

/// This contains the runtime configuration of LyX
class LyXRC
{
public:
	enum LyXRCTags {
		RC_ACCEPT_COMPOUND = 1,
		RC_ALT_LANG,
		RC_AUTOCORRECTION_MATH,
		RC_AUTOREGIONDELETE,
		RC_AUTORESET_OPTIONS,
		RC_AUTOSAVE,
		RC_AUTO_NUMBER,
		RC_BACKUPDIR_PATH,
		RC_BIBTEX_ALTERNATIVES,
		RC_BIBTEX_COMMAND,
		RC_BINDFILE,
		RC_BOOKMARKS_VISIBILITY,
		RC_CHECKLASTFILES,
		RC_CHKTEX_COMMAND,
		RC_CITATION_SEARCH,
		RC_CITATION_SEARCH_PATTERN,
		RC_CITATION_SEARCH_VIEW,
		RC_COMPLETION_CURSOR_TEXT,
		RC_COMPLETION_INLINE_DELAY,
		RC_COMPLETION_INLINE_MATH,
		RC_COMPLETION_INLINE_TEXT,
		RC_COMPLETION_INLINE_DOTS,
		RC_COMPLETION_MINLENGTH,
		RC_COMPLETION_POPUP_DELAY,
		RC_COMPLETION_POPUP_MATH,
		RC_COMPLETION_POPUP_TEXT,
		RC_COMPLETION_POPUP_AFTER_COMPLETE,
		RC_CONVERTER,
		RC_CONVERTER_CACHE_MAXAGE,
		RC_COPIER,
		RC_CT_ADDITIONS_UNDERLINED,
		RC_CT_MARKUP_COPIED,
		RC_CURSOR_FOLLOWS_SCROLLBAR,
		RC_CURSOR_WIDTH,
		RC_DEFAULT_DECIMAL_SEP,
		RC_DEFAULT_LENGTH_UNIT,
		RC_DEFAULT_OTF_VIEW_FORMAT,
		RC_DEFAULT_PLATEX_VIEW_FORMAT,
		RC_DEFAULT_VIEW_FORMAT,
		RC_DEFFILE,
		RC_DIALOGS_ICONIFY_WITH_MAIN,
		RC_DISPLAY_GRAPHICS,
		RC_DOCUMENTPATH,
		RC_EDITOR_ALTERNATIVES,
		RC_ESC_CHARS,
		RC_EXAMPLEPATH,
		RC_EXPORT_OVERWRITE,
		RC_FILEFORMAT,
		RC_FORWARD_SEARCH_DVI,
		RC_FORWARD_SEARCH_PDF,
		RC_FULL_SCREEN_LIMIT,
		RC_FULL_SCREEN_SCROLLBAR,
		RC_FULL_SCREEN_STATUSBAR,
		RC_FULL_SCREEN_TABBAR,
		RC_FULL_SCREEN_MENUBAR,
		RC_FULL_SCREEN_TOOLBARS,
		RC_FULL_SCREEN_WIDTH,
		RC_GEOMETRY_SESSION,
		RC_GROUP_LAYOUTS,
		RC_GUI_LANGUAGE,
		RC_HUNSPELLDIR_PATH,
		RC_ICON_SET,
		RC_INDEX_ALTERNATIVES,
		RC_INDEX_COMMAND,
		RC_INPUT,
		RC_JBIBTEX_ALTERNATIVES,
		RC_JBIBTEX_COMMAND,
		RC_JINDEX_COMMAND,
		RC_KBMAP,
		RC_KBMAP_PRIMARY,
		RC_KBMAP_SECONDARY,
		RC_LANGUAGE_AUTO_BEGIN,
		RC_LANGUAGE_AUTO_END,
		RC_LANGUAGE_COMMAND_BEGIN,
		RC_LANGUAGE_COMMAND_END,
		RC_LANGUAGE_COMMAND_LOCAL,
		RC_LANGUAGE_GLOBAL_OPTIONS,
		RC_LANGUAGE_CUSTOM_PACKAGE,
		RC_LANGUAGE_PACKAGE_SELECTION,
		RC_LOADSESSION,
		RC_LYXRCFORMAT,
		RC_MACRO_EDIT_STYLE,
		RC_MAC_DONTSWAP_CTRL_META,
		RC_MAC_LIKE_CURSOR_MOVEMENT,
		RC_MAKE_BACKUP,
		RC_MARK_FOREIGN_LANGUAGE,
		RC_MOUSE_WHEEL_SPEED,
		RC_MOUSE_MIDDLEBUTTON_PASTE,
		RC_NOMENCL_COMMAND,
		RC_NUMLASTFILES,
		RC_OPEN_BUFFERS_IN_TABS,
		RC_PARAGRAPH_MARKERS,
		RC_PATH_PREFIX,
		RC_PLAINTEXT_LINELEN,
		RC_PREVIEW,
		RC_PREVIEW_HASHED_LABELS,
		RC_PREVIEW_SCALE_FACTOR,
		RC_PRINTLANDSCAPEFLAG,
		RC_PRINTPAPERDIMENSIONFLAG,
		RC_PRINTPAPERFLAG,
		RC_PYGMENTIZE_COMMAND,
		RC_RESPECT_OS_KBD_LANGUAGE,
		RC_SAVE_COMPRESSED,
		RC_SAVE_ORIGIN,
		RC_SCREEN_DPI,
		RC_SCREEN_FONT_ROMAN,
		RC_SCREEN_FONT_ROMAN_FOUNDRY,
		RC_SCREEN_FONT_SANS,
		RC_SCREEN_FONT_SANS_FOUNDRY,
		RC_SCREEN_FONT_SCALABLE,
		RC_SCREEN_FONT_SIZES,
		RC_SCREEN_FONT_TYPEWRITER,
		RC_SCREEN_FONT_TYPEWRITER_FOUNDRY,
		RC_SCREEN_ZOOM,
		RC_SCROLL_BELOW_DOCUMENT,
		RC_SCROLL_WHEEL_ZOOM,
		RC_SERVERPIPE,
		RC_SET_COLOR,
		RC_SHOW_BANNER,
		RC_SINGLE_CLOSE_TAB_BUTTON,
		RC_SINGLE_INSTANCE,
		RC_SORT_LAYOUTS,
		RC_SPELLCHECK_CONTINUOUSLY,
		RC_SPELLCHECK_NOTES,
		RC_SPELLCHECKER,
		RC_SPLITINDEX_COMMAND,
		RC_TEMPDIRPATH,
		RC_TEMPLATEPATH,
		RC_TEX_ALLOWS_SPACES,
		RC_TEX_EXPECTS_WINDOWS_PATHS,
		RC_TEXINPUTS_PREFIX,
		RC_THESAURUSDIRPATH,
		RC_UIFILE,
		RC_USELASTFILEPOS,
		RC_USER_EMAIL,
		RC_USER_INITIALS,
		RC_USER_NAME,
		RC_USE_CONVERTER_CACHE,
		RC_USE_CONVERTER_NEEDAUTH_FORBIDDEN,
		RC_USE_CONVERTER_NEEDAUTH,
		RC_USE_NATIVE_FILEDIALOG,
		RC_USE_SYSTEM_COLORS,
		RC_USE_TOOLTIP,
		RC_USE_SYSTEM_THEME_ICONS,
		RC_VIEWDVI_PAPEROPTION,
		RC_VIEWER,
		RC_VIEWER_ALTERNATIVES,
		RC_VISUAL_CURSOR,
		RC_CLOSE_BUFFER_WITH_LAST_VIEW,
		RC_LAST
	};

	///
	LyXRC() : user_name(support::user_name()),
	          user_email(support::user_email()) // always empty
		{}

	/// \param check_format: whether to try to convert the file format,
	/// if it is not current. this should only be true, really, for the
	/// user's own preferences file.
	bool read(support::FileName const & filename, bool check_format);
	///
	bool read(std::istream &);
private:
	enum ReturnValues {
		ReadOK,
		ReadError,
		FormatMismatch
	};
	///
	ReturnValues read(Lexer &, bool check_format);
public:
	///
	typedef std::set<std::string> CommandSet;
	/// maps a format to a set of commands that can be used to
	/// edit or view it.
	typedef std::map<std::string, CommandSet> Alternatives;
	///
	void write(support::FileName const & filename,
		   bool ignore_system_lyxrc) const;
	/// write rc. If a specific tag is given, only output that one.
	void write(std::ostream & os,
		   bool ignore_system_lyxrc,
		   std::string const & tag = std::string()) const;
	///
	void print() const;
	///
	std::set<std::string> getRCs();
	// FIXME unused (was used for xforms. Do we still need this?)
	//static docstring const getDescription(LyXRCTags);
	///
	std::string bind_file = "cua";
	///
	std::string def_file = "default";
	///
	std::string ui_file = "default";
	///
	std::string print_landscape_flag = "-t landscape";
	///
	std::string print_paper_flag = "-t";
	///
	std::string print_paper_dimension_flag = "-T";
	/// option for telling the dvi viewer about the paper size
	std::string view_dvi_paper_option;
	/// default paper size for local xdvi/dvips/ghostview/whatever
	/// command to run chktex incl. options
	std::string chktex_command = "chktex -n1 -n3 -n6 -n9 -n22 -n25 -n30 -n38";
	/// Use external script to search for file corresponding to a 
	/// given citation.
	bool citation_search = false;
	/// Name of external script which searches for file corresponding to a 
	/// given citation. At this moment only script lyxpaperview.p is supported
	/// (search for pdf or ps based on specific items)
	std::string citation_search_view;
	/// Items to search for in citation_search_view
	std::string citation_search_pattern = "%year% %abbrvciteauthor%";
	/// all available commands to run bibtex incl. options
	CommandSet bibtex_alternatives;
	/// command to run bibtex incl. options
	std::string bibtex_command = "automatic";
	/// command to run japanese bibtex incl. options
	std::string jbibtex_command;
	/// all available commands to run japanese bibtex incl. options
	CommandSet jbibtex_alternatives;
	/// all available index commands incl. options
	CommandSet index_alternatives;
	/// command to run makeindex incl. options or other index programs
	std::string index_command = "makeindex -c -q";
	/// command to run japanese index program incl. options
	std::string jindex_command;
	/// command to generate multiple indices
	std::string splitindex_command;
	/// command to run makeindex incl. options for nomencl
	std::string nomencl_command = "makeindex -s nomencl.ist";
	/// command to run the python pygments syntax highlighter
	std::string pygmentize_command;
	///
	std::string document_path;
	///
	std::string example_path;
	///
	std::string template_path;
	///
	std::string tempdir_path;
	///
	std::string thesaurusdir_path;
	///
	std::string hunspelldir_path;
	///
	bool auto_region_delete = true;
	/// enable middle-mouse-button paste
	bool mouse_middlebutton_paste = true;
	///
	bool auto_reset_options = false;
	/// flag telling whether lastfiles should be checked for existance
	bool check_lastfiles = true;
	/// maximal number of lastfiles
	unsigned int num_lastfiles = 20;
	/// whether or not go to saved position when opening a file
	bool use_lastfilepos = true;
	/// load files from last session automatically
	bool load_session = false;
	/// do we save new documents as compressed by default
	bool save_compressed = false;
	/// whether or not to save the document dir in the file
	bool save_origin = false;
	/// shall a backup file be created
	bool make_backup = true;
	/// A directory for storing backup files
	std::string backupdir_path;
	/// Whether or not save/restore session information
	/// like windows position and geometry.
	bool allow_geometry_session = true;
	/// Scrolling speed of the mouse wheel
	double mouse_wheel_speed = 1.0;
	/// Default zoom factor for screen fonts
	int defaultZoom = 150;
	/// Actual zoom factor for screen fonts
	/// (default zoom plus buffer zoom factor)
	/// Do not set directly. Use GuiView::setCurrentZoom()
	int currentZoom = 150;
	/// Screen font sizes in points for each font size
	std::string font_sizes[10] = { "5.0", "7.0", "8.0", "9.0", "10.0",
	                               "12.0", "14.4", "17.26", "20.74", "24.88"};
	/// Allow the use of scalable fonts? Default is yes.
	bool use_scalable_fonts = true;
	/// DPI of monitor
	unsigned int dpi = 75;
	///
	std::string roman_font_name;
	///
	std::string sans_font_name;
	///
	std::string typewriter_font_name;
	///
	std::string roman_font_foundry;
	///
	std::string sans_font_foundry;
	///
	std::string typewriter_font_foundry;
	///
	unsigned int autosave = 300;
	///
	unsigned int plaintext_linelen = 65;
	/// End of paragraph markers?
	bool paragraph_markers = false;
	/// Use tooltips?
	bool use_tooltip = true;
	/// Use the colors from current system theme?
	bool use_system_colors = false;
	/// use native file dialog or our own ?
	bool use_native_filedialog = true;
	/// Spellchecker engine: aspell, hunspell, etc
	std::string spellchecker
#if defined(USE_MACOSX_PACKAGING)
		= "native";
#elif defined(USE_ENCHANT)
		= "enchant";
#elif defined(USE_ASPELL)
		= "aspell";
#elif defined(USE_HUNSPELL)
		= "hunspell";
#else
		= "aspell";
#endif
	/// Alternate language for spellchecker
	std::string spellchecker_alt_lang;
	/// Escape characters
	std::string spellchecker_esc_chars;
	/// Accept compound words in spellchecker?
	bool spellchecker_accept_compound = false;
	/// spellcheck continuously?
	bool spellcheck_continuously = true;
	/// spellcheck notes and comments?
	bool spellcheck_notes = true;
	///
	bool use_kbmap = false;
	///
	std::string primary_kbmap;
	///
	std::string secondary_kbmap;
	///
	std::string lyxpipes;
	///
	std::string language_custom_package = "\\usepackage{babel}";
	///
	bool language_auto_begin = true;
	///
	bool language_auto_end = true;
	///
	std::string language_command_begin = "\\selectlanguage{$$lang}";
	///
	std::string language_command_end;
	///
	std::string language_command_local = "\\foreignlanguage{$$lang}{";
	///
	bool language_global_options = true;
	///
	enum LangPackageSelection {
		LP_AUTO = 0,
		LP_BABEL,
		LP_CUSTOM,
		LP_NONE
	};
	///
	LangPackageSelection language_package_selection = LP_AUTO;
	/// bidi cursor movement: true = visual, false = logical
	bool visual_cursor = false;
	///
	bool auto_number = true;
	///
	bool mark_foreign_language = true;
	///
	std::string gui_language = "auto";
	///
	bool respect_os_kbd_language = false;
	///
	std::string default_otf_view_format = "pdf4";
	///
	std::string default_platex_view_format = "pdf3";
	///
	std::string default_view_format = "pdf2";
	/// all available viewers
	Alternatives viewer_alternatives;
	/// all available editors
	Alternatives editor_alternatives;
	///
	bool mac_dontswap_ctrl_meta = false;
	///
	bool mac_like_cursor_movement = false;
	///
	bool cursor_follows_scrollbar = false;
	///
	bool ct_additions_underlined = true;
	///
	bool ct_markup_copied = false;
	///
	bool scroll_below_document = false;
	///
	enum MacroEditStyle {
		MACRO_EDIT_INLINE_BOX = 0,
		MACRO_EDIT_INLINE,
		MACRO_EDIT_LIST
	};
	///
	MacroEditStyle macro_edit_style = MACRO_EDIT_INLINE_BOX;
	///
	bool dialogs_iconify_with_main = false;
	///
	bool display_graphics = true;
	///
	bool show_banner = true;
	///
	enum PreviewStatus {
		PREVIEW_OFF,
		PREVIEW_NO_MATH,
		PREVIEW_ON
	};
	///
	PreviewStatus preview = PREVIEW_OFF;
	///
	bool preview_hashed_labels = false;
	///
	double preview_scale_factor = 1.0;
	/// user name
	std::string user_name; // set in constructor
	/// user email
	std::string user_email; // set in constructor (empty for now)
	/// user initials
	std::string user_initials;
	/// icon set name
	std::string icon_set;
	/// whether to use the icons from the theme
	bool use_system_theme_icons = false;
	/// True if the TeX engine cannot handle posix paths
	bool windows_style_tex_paths = false;
	/// True if the TeX engine can handle file names containing spaces
	bool tex_allows_spaces = false;
	/** Prepend paths to the PATH environment variable.
	 *  The string is input, stored and output in native format.
	 */
	std::string path_prefix;
	/** Prepend paths to the TEXINPUTS environment variable.
	 *  The string is input, stored and output in native format.
	 *  A '.' here stands for the current document directory.
	 */
	std::string texinputs_prefix = ".";
	/// Use the cache for file converters?
	bool use_converter_cache = true;
	/// Forbid use of external converters with 'needauth' option
	bool use_converter_needauth_forbidden = true;
	/// Ask user before calling external converters with 'needauth' option
	bool use_converter_needauth = true;
	/// The maximum age of cache files in seconds
	unsigned int converter_cache_maxage = 6 * 30 * 24 * 3600; // 6 months;
	/// Sort layouts alphabetically
	bool sort_layouts = false;
	/// Group layout by their category
	bool group_layouts = true;
	/// Toggle toolbars in fullscreen mode?
	bool full_screen_toolbars = true;
	/// Toggle scrollbar in fullscreen mode?
	bool full_screen_scrollbar = true;
	/// Toggle tabbar in fullscreen mode?
	bool full_screen_tabbar = true;
	/// Toggle menubar in fullscreen mode?
	bool full_screen_menubar = true;
	/// Toggle statusbar in fullscreen mode?
	bool full_screen_statusbar = true;
	/// Limit the text width?
	bool full_screen_limit = false;
	/// Width of limited screen (in pixels) in fullscreen mode
	int full_screen_width = 700;
	///
	bool completion_cursor_text = true;
	///
	double completion_inline_delay = 0.2;
	///
	bool completion_inline_math = true;
	///
	bool completion_inline_text = false;
	///
	int completion_inline_dots = -1;
	/// minimum length of words to complete
	unsigned int completion_minlength = 6;
	///
	double completion_popup_delay = 2.0;
	///
	bool completion_popup_math = true;
	///
	bool completion_popup_text = false;
	///
	bool completion_popup_after_complete = true;
	///
	bool autocorrection_math = false;
	///
	bool open_buffers_in_tabs = true;
	///
	bool single_close_tab_button = false;
	///
	bool single_instance = true;
	///
	std::string forward_search_dvi;
	///
	std::string forward_search_pdf;
	///
	int export_overwrite = NO_FILES;
	/// Default decimal point when aligning table columns on decimal
	std::string default_decimal_sep = "locale";
	///
	Length::UNIT default_length_unit = Length::CM;
	///
	enum ScrollWheelZoom {
		SCROLL_WHEEL_ZOOM_OFF,
		SCROLL_WHEEL_ZOOM_CTRL,
		SCROLL_WHEEL_ZOOM_SHIFT,
		SCROLL_WHEEL_ZOOM_ALT
	};
	///
	ScrollWheelZoom scroll_wheel_zoom = SCROLL_WHEEL_ZOOM_CTRL;
	// FIXME: should be caret_width
	///
	int cursor_width = 0;
	/// One of: yes, no, ask
	std::string close_buffer_with_last_view = "yes";
	enum BookmarksVisibility {
		BMK_NONE,
		BMK_MARGIN,
		BMK_INLINE
	};

	///
	BookmarksVisibility bookmarks_visibility = BMK_NONE;
};


void actOnUpdatedPrefs(LyXRC const & lyxrc_orig, LyXRC const & lyxrc_new);

///
extern LyXRC lyxrc;
///
extern LyXRC system_lyxrc;

// used by at least frontends/qt/GuiPref.cpp
const long maxlastfiles = 50;

} // namespace lyx

#endif
