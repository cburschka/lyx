// -*- C++ -*-
/**
 * \file lyxrc.h
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
#include "graphics/GraphicsTypes.h"

#include <iosfwd>
#include <string>

class LyXLex;

/// This contains the runtime configuration of LyX
class LyXRC //: public noncopyable {
// after 1.1.6 I will use a LyXRCStruct here and then this can be made
// noncopyable again.  For now I want to minimise changes.  ARRae 20001010
{
public:
enum LyXRCTags {
	RC_FONT_ENCODING = 1,
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
	RC_SCREEN_DPI,
	RC_SCREEN_ZOOM,
	RC_SCREEN_FONT_SIZES,
	RC_SCREEN_FONT_ROMAN,
	RC_SCREEN_FONT_SANS,
	RC_SCREEN_FONT_TYPEWRITER,
	RC_SCREEN_FONT_ROMAN_FOUNDRY,
	RC_SCREEN_FONT_SANS_FOUNDRY,
	RC_SCREEN_FONT_TYPEWRITER_FOUNDRY,
	RC_SCREEN_FONT_ENCODING,
	RC_POPUP_BOLD_FONT,
	RC_POPUP_NORMAL_FONT,
	RC_POPUP_FONT_ENCODING,
	RC_SET_COLOR,
	RC_AUTOSAVE,
	RC_DOCUMENTPATH,
	RC_TEMPLATEPATH,
	RC_TEMPDIRPATH,
	RC_USETEMPDIR,
	RC_LASTFILES,
	RC_AUTOREGIONDELETE,
	RC_AUTORESET_OPTIONS,
	RC_SERVERPIPE,
	RC_INPUT,
	RC_BINDFILE,
	RC_UIFILE,
	RC_KBMAP,
	RC_KBMAP_PRIMARY,
	RC_KBMAP_SECONDARY,
	RC_ASCIIROFF_COMMAND,
	RC_ASCII_LINELEN,
	RC_NUMLASTFILES,
	RC_CHECKLASTFILES,
	RC_VIEWDVI_PAPEROPTION,
	RC_DEFAULT_PAPERSIZE,
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
	RC_BIBTEX_COMMAND,
	RC_CURSOR_FOLLOWS_SCROLLBAR,
	RC_DIALOGS_ICONIFY_WITH_MAIN,
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
	RC_LANGUAGE_COMMAND_LOCAL,
	RC_LANGUAGE_GLOBAL_OPTIONS,
	RC_LANGUAGE_USE_BABEL,
	RC_DATE_INSERT_FORMAT,
	RC_SHOW_BANNER,
	RC_WHEEL_JUMP,
	RC_CONVERTER,
	RC_VIEWER,
	RC_FORMAT,
	RC_DEFAULT_LANGUAGE,
	RC_LABEL_INIT_LENGTH,
	RC_DISPLAY_GRAPHICS,
	RC_PREVIEW,
	RC_PREVIEW_HASHED_LABELS,
	RC_PREVIEW_SCALE_FACTOR,
	RC_USE_SPELL_LIB,
	RC_USER_NAME,
	RC_USER_EMAIL,
	RC_LAST
};


	///
	LyXRC();
	///
	void setDefaults();
	///
	int read(std::string const & filename);
	///
	int read(std::istream &);
private:
	///
	int read(LyXLex &);
public:
	///
	void write(std::string const & filename) const;
	///
	void write(std::ostream & os) const;
	///
	void print() const;
	///
	static std::string const getDescription(LyXRCTags);
	///
	std::string bind_file;
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
	///
	std::string document_path;
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
	/// filename for lastfiles file
	std::string lastfiles;
	/// maximal number of lastfiles
	unsigned int num_lastfiles;
	/// shall a backup file be created
	bool make_backup;
	/// A directory for storing backup files
	std::string backupdir_path;
	/// Zoom factor for screen fonts
	unsigned int zoom;
	/// parameter for button_4 and button_5 (scrollwheel)
	unsigned int wheel_jump;
	/// Screen font sizes in points for each font size
	float font_sizes[10];
	/// Allow the use of scalable fonts? Default is yes.
	bool use_scalable_fonts;
	/// DPI of monitor
	float dpi;
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
	std::string popup_bold_font;
	///
	std::string popup_normal_font;
	///
	std::string font_norm;
	///
	enum FontEncoding {
		///
		ISO_10646_1,
		///
		ISO_8859_1,
		///
		ISO_8859_3,
		///
		ISO_8859_4,
		///
		ISO_8859_6_8,
		///
		ISO_8859_9,
		///
		ISO_8859_15,
		///
		OTHER_ENCODING
	};
	///
	FontEncoding font_norm_type;
	///
	void set_font_norm_type();
	///
	std::string popup_font_encoding;
	///
	unsigned int autosave;
	///
	std::string ascii_roff_command;
	///
	unsigned int ascii_linelen;
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
	///
	bool auto_number;
	///
	bool mark_foreign_language;
	///
	std::string default_language;
	///
	bool cursor_follows_scrollbar;
	///
	bool dialogs_iconify_with_main;
	///
	int label_init_length;
	///
	lyx::graphics::DisplayType display_graphics;
	///
	bool show_banner;
	///
	bool preview;
	///
	bool preview_hashed_labels;
	///
	float preview_scale_factor;
	/// user name
	std::string user_name;
	/// user email
	std::string user_email;
};

///
extern LyXRC lyxrc;
///
extern LyXRC system_lyxrc;
#endif
