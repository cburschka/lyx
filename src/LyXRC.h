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

#include "paper.h"

#include "support/strfwd.h"

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
		RC_PLAINTEXT_ROFF_COMMAND,
		RC_PLAINTEXT_LINELEN,
		RC_AUTOREGIONDELETE,
		RC_AUTORESET_OPTIONS,
		RC_AUTOSAVE,
		RC_AUTO_NUMBER,
		RC_BACKUPDIR_PATH,
		RC_BIBTEX_COMMAND,
		RC_BINDFILE,
		RC_CHECKLASTFILES,
		RC_CHKTEX_COMMAND,
		RC_COMPLETION_CURSOR_TEXT,
		RC_COMPLETION_INLINE_DELAY,
		RC_COMPLETION_INLINE_MATH,
		RC_COMPLETION_INLINE_TEXT,
		RC_COMPLETION_INLINE_DOTS,
		RC_COMPLETION_POPUP_DELAY,
		RC_COMPLETION_POPUP_MATH,
		RC_COMPLETION_POPUP_TEXT,
		RC_COMPLETION_POPUP_AFTER_COMPLETE,
		RC_CONVERTER,
		RC_CONVERTER_CACHE_MAXAGE,
		RC_COPIER,
		RC_CURSOR_FOLLOWS_SCROLLBAR,
		RC_CUSTOM_EXPORT_COMMAND,
		RC_CUSTOM_EXPORT_FORMAT,
		RC_DATE_INSERT_FORMAT,
		RC_DEFFILE,
		RC_DEFAULT_LANGUAGE,
		RC_GUI_LANGUAGE,
		RC_DEFAULT_PAPERSIZE,
		RC_DIALOGS_ICONIFY_WITH_MAIN,
		RC_DISPLAY_GRAPHICS,
		RC_DOCUMENTPATH,
		RC_ESC_CHARS,
		RC_EXAMPLEPATH,
		RC_FONT_ENCODING,
		RC_FORMAT,
		RC_FULL_SCREEN_LIMIT,
		RC_FULL_SCREEN_SCROLLBAR,
		RC_FULL_SCREEN_TABBAR,
		RC_FULL_SCREEN_TOOLBARS,
		RC_FULL_SCREEN_WIDTH,
		RC_GEOMETRY_SESSION,
		RC_GROUP_LAYOUTS,
		RC_INDEX_COMMAND,
		RC_INPUT,
		RC_KBMAP,
		RC_KBMAP_PRIMARY,
		RC_KBMAP_SECONDARY,
		RC_LABEL_INIT_LENGTH,
		RC_LANGUAGE_AUTO_BEGIN,
		RC_LANGUAGE_AUTO_END,
		RC_LANGUAGE_COMMAND_BEGIN,
		RC_LANGUAGE_COMMAND_END,
		RC_LANGUAGE_COMMAND_LOCAL,
		RC_LANGUAGE_GLOBAL_OPTIONS,
		RC_LANGUAGE_PACKAGE,
		RC_LANGUAGE_USE_BABEL,
		RC_LOADSESSION,
		RC_MACRO_EDIT_STYLE,
		RC_MAKE_BACKUP,
		RC_MARK_FOREIGN_LANGUAGE,
		RC_MOUSE_WHEEL_SPEED,
		RC_NUMLASTFILES,
		RC_PATH_PREFIX,
		RC_PERS_DICT,
		RC_PREVIEW,
		RC_PREVIEW_HASHED_LABELS,
		RC_PREVIEW_SCALE_FACTOR,
		RC_PRINTCOLLCOPIESFLAG,
		RC_PRINTCOPIESFLAG,
		RC_PRINTER,
		RC_PRINTEVENPAGEFLAG,
		RC_PRINTEXSTRAOPTIONS,
		RC_PRINTFILEEXTENSION,
		RC_PRINTLANDSCAPEFLAG,
		RC_PRINTODDPAGEFLAG,
		RC_PRINTPAGERANGEFLAG,
		RC_PRINTPAPERDIMENSIONFLAG,
		RC_PRINTPAPERFLAG,
		RC_PRINTREVERSEFLAG,
		RC_PRINTSPOOL_COMMAND,
		RC_PRINTSPOOL_PRINTERPREFIX,
		RC_PRINTTOFILE,
		RC_PRINTTOPRINTER,
		RC_PRINT_ADAPTOUTPUT,
		RC_PRINT_COMMAND,
		RC_RTL_SUPPORT,
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
		RC_SERVERPIPE,
		RC_SET_COLOR,
		RC_SHOW_BANNER,
		RC_OPEN_BUFFERS_IN_TABS,
		RC_SORT_LAYOUTS,
		RC_SPELL_COMMAND,
		RC_TEMPDIRPATH,
		RC_TEMPLATEPATH,
		RC_TEX_ALLOWS_SPACES,
		RC_TEX_EXPECTS_WINDOWS_PATHS,
		RC_UIFILE,
		RC_USELASTFILEPOS,
		RC_USER_EMAIL,
		RC_USER_NAME,
		RC_USETEMPDIR,
		RC_USE_ALT_LANG,
		RC_USE_CONVERTER_CACHE,
		RC_USE_ESC_CHARS,
		RC_USE_INP_ENC,
		RC_USE_PERS_DICT,
		RC_USE_TOOLTIP,
		RC_USE_PIXMAP_CACHE,
		RC_USE_SPELL_LIB,
		RC_VIEWDVI_PAPEROPTION,
		RC_VIEWER,
		RC_VISUAL_CURSOR,
		RC_LAST
	};

	///
	LyXRC();
	///
	void setDefaults();
	///
	int read(support::FileName const & filename);
	///
	int read(std::istream &);
private:
	///
	int read(Lexer &);
public:
	///
	void write(support::FileName const & filename,
		   bool ignore_system_lyxrc) const;
	/// write rc. If a specific tag is given, only output that one.
	void write(std::ostream & os,
		   bool ignore_system_lyxrc, 
		   std::string const & tag = std::string()) const;
	///
	void print() const;
	// FIXME unused (was used for xforms. Do we still need this?)
	//static docstring const getDescription(LyXRCTags);
	///
	std::string bind_file;
	///
	std::string def_file;
	///
	std::string ui_file;
	///
	std::string printer;
	///
	std::string print_command;
	///
	std::string print_evenpage_flag;
	///
	std::string print_oddpage_flag;
	///
	std::string print_pagerange_flag;
	///
	std::string print_copies_flag;
	///
	std::string print_collcopies_flag;
	///
	std::string print_reverse_flag;
	///
	std::string print_landscape_flag;
	///
	std::string print_to_printer;
	///
	bool print_adapt_output;
	///
	std::string print_to_file;
	///
	std::string print_file_extension;
	///
	std::string print_extra_options;
	///
	std::string print_spool_command;
	///
	std::string print_spool_printerprefix;
	///
	std::string print_paper_flag;
	///
	std::string print_paper_dimension_flag;
	///
	std::string custom_export_command;
	///
	std::string custom_export_format;
	/// option for telling the dvi viewer about the paper size
	std::string view_dvi_paper_option;
	/// default paper size for local xdvi/dvips/ghostview/whatever
	PAPER_SIZE default_papersize;
	/// command to run chktex incl. options
	std::string chktex_command;
	/// command to run bibtex incl. options
	std::string bibtex_command;
	/// command to run makeindex incl. options or other index programs
	std::string index_command;
	///
	std::string document_path;
	///
	std::string example_path;
	///
	std::string template_path;
	///
	std::string tempdir_path;
	///
	bool auto_region_delete;
	/// flag telling whether lastfiles should be checked for existance
	bool auto_reset_options;
	///
	bool check_lastfiles;
	/// maximal number of lastfiles
	unsigned int num_lastfiles;
	/// whether or not go to saved position when opening a file
	bool use_lastfilepos;
	/// load files from last session automatically
	bool load_session;
	/// shall a backup file be created
	bool make_backup;
	/// A directory for storing backup files
	std::string backupdir_path;
	/// Whether or not save/restore session information
	/// like windows position and geometry.
	bool allow_geometry_session;
	/// Scrolling speed of the mouse wheel
	double mouse_wheel_speed;
	/// Zoom factor for screen fonts
	unsigned int zoom;
	/// Screen font sizes in points for each font size
	std::string font_sizes[10];
	/// Allow the use of scalable fonts? Default is yes.
	bool use_scalable_fonts;
	/// DPI of monitor
	unsigned int dpi;
	///
	std::string fontenc;
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
	unsigned int autosave;
	///
	std::string plaintext_roff_command;
	///
	unsigned int plaintext_linelen;
	/// use library instead of process
	bool use_spell_lib;
	/// Ispell command
	std::string isp_command;
	/// Accept compound words in spellchecker?
	bool isp_accept_compound;
	/// Pass input encoding switch to ispell?
	bool isp_use_input_encoding;
	/// Use alternate language?
	bool isp_use_alt_lang;
	/// Use personal dictionary?
	bool isp_use_pers_dict;
	/// Use tooltips?
	bool use_tooltip;
	/// Use pixmap cache?
	bool use_pixmap_cache;
	/// Use escape chars?
	bool isp_use_esc_chars;
	/// Alternate language for ispell
	std::string isp_alt_lang;
	/// Alternate personal dictionary file for ispell
	std::string isp_pers_dict;
	/// Escape characters
	std::string isp_esc_chars;
	///
	bool use_kbmap;
	///
	std::string primary_kbmap;
	///
	std::string secondary_kbmap;
	///
	std::string lyxpipes;
	///
	std::string date_insert_format;
	///
	std::string language_package;
	///
	bool language_auto_begin;
	///
	bool language_auto_end;
	///
	std::string language_command_begin;
	///
	std::string language_command_end;
	///
	std::string language_command_local;
	///
	bool language_global_options;
	///
	bool language_use_babel;
	///
	bool rtl_support;
	/// bidi cursor movement: true = visual, false = logical
	bool visual_cursor;
	///
	bool auto_number;
	///
	bool mark_foreign_language;
	///
	std::string default_language;
	///
	std::string gui_language;
	///
	bool cursor_follows_scrollbar;
	///
	enum MacroEditStyle {
		MACRO_EDIT_INLINE_BOX = 0,
		MACRO_EDIT_INLINE,
		MACRO_EDIT_LIST
	};
	///
	MacroEditStyle macro_edit_style;
	///
	bool dialogs_iconify_with_main;
	///
	int label_init_length;
	///
	bool display_graphics;
	///
	bool show_banner;
	///
	enum PreviewStatus {
		PREVIEW_OFF,
		PREVIEW_NO_MATH,
		PREVIEW_ON
	};
	///
	PreviewStatus preview;
	///
	bool preview_hashed_labels;
	///
	std::string preview_scale_factor;
	/// user name
	std::string user_name;
	/// user email
	std::string user_email;
	/// True if the TeX engine cannot handle posix paths
	bool windows_style_tex_paths;
	/// True if the TeX engine can handle file names containing spaces
	bool tex_allows_spaces;
	/** Prepend paths to the PATH environment variable.
	 *  The string is input, stored and output in native format.
	 */
	std::string path_prefix;
	/// Use the cache for file converters?
	bool use_converter_cache;
	/// The maximum age of cache files in seconds
	unsigned int converter_cache_maxage;
	/// Sort layouts alphabetically
	bool sort_layouts;
	/// Group layout by their category
	bool group_layouts;
	/// Toggle toolbars in fullscreen mode?
	bool full_screen_toolbars;
	/// Toggle scrollbar in fullscreen mode?
	bool full_screen_scrollbar;
	/// Toggle tabbar in fullscreen mode?
	bool full_screen_tabbar;
	/// Limit the text width?
	bool full_screen_limit;
	/// Width of limited screen (in pixels) in fullscreen mode
	int full_screen_width;
	///
	bool completion_cursor_text;
	///
	double completion_inline_delay;
	///
	bool completion_inline_math;
	///
	bool completion_inline_text;
	///
	int completion_inline_dots;
	///
	double completion_popup_delay;
	///
	bool completion_popup_math;
	///
	bool completion_popup_text;
	///
	bool completion_popup_after_complete;
	///
	bool open_buffers_in_tabs;
};


/** \c LyXRC_PreviewStatus is a wrapper for LyXRC::PreviewStatus.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose LyXRC.h.
 */
class LyXRC_PreviewStatus {
	LyXRC::PreviewStatus val_;
public:
	LyXRC_PreviewStatus(LyXRC::PreviewStatus val) : val_(val) {}
	operator LyXRC::PreviewStatus() const { return val_; }
};


///
extern LyXRC lyxrc;
///
extern LyXRC system_lyxrc;

} // namespace lyx

#endif
