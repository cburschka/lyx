// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXRC_H
#define LYXRC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "bufferparams.h" // Just to get the enum BufferParams::PAPER_SIZE (sic)

// #include <boost/utility.hpp>

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
	RC_BIND,
	RC_OVERRIDE_X_DEADKEYS,
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
	RC_PS_COMMAND,
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
#ifdef USE_PSPELL
	RC_USE_PSPELL,
#endif
	RC_LAST
};


	///
	LyXRC();
	///
	void setDefaults();
	///
	int read(string const & filename);
	///
	void readBindFileIfNeeded();
	///
	void write(string const & filename) const;
	///
	void print() const;
	///
	void output(std::ostream & os) const;
	///
	static string const getDescription(LyXRCTags);
	///
	string bind_file;
	///
	string ui_file;
	///
	string printer;
	///
	string print_command;
	///
	string print_evenpage_flag;
	///
	string print_oddpage_flag;
	///
	string print_pagerange_flag;
	///
	string print_copies_flag;
	///
	string print_collcopies_flag;
	///
	string print_reverse_flag;
	///
	string print_landscape_flag;
	///
	string print_to_printer;
	///
	bool print_adapt_output;
	///
	string print_to_file;
	///
	string print_file_extension;
	///
	string print_extra_options;
	///
	string print_spool_command;
	///
	string print_spool_printerprefix;
	///
	string print_paper_flag;
	///
	string print_paper_dimension_flag;
	///
	string custom_export_command;
	///
	string custom_export_format;
	///
	bool pdf_mode;
	/// postscript interpreter (in general "gs", if it is installed)
	string ps_command;
	/// option for telling the dvi viewer about the paper size
	string view_dvi_paper_option;
	/// default paper size for local xdvi/dvips/ghostview/whatever
	BufferParams::PAPER_SIZE default_papersize;
	/// command to run chktex incl. options
	string chktex_command;
	///
	string document_path;
	///
	string template_path;
	///
	string tempdir_path;
	///
	bool use_tempdir;
	///
	bool auto_region_delete;
	/// flag telling whether lastfiles should be checked for existance
	bool auto_reset_options;
	///
	bool check_lastfiles;
	/// filename for lastfiles file
	string lastfiles;
	/// maximal number of lastfiles
	unsigned int num_lastfiles;
	/// shall a backup file be created
	bool make_backup;
	/// A directory for storing backup files
	string backupdir_path;
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
	/// Whether lyx should handle deadkeys by itself
	bool override_x_deadkeys;
	///
	string fontenc;
	///
	string roman_font_name;
	///
	string sans_font_name;
	///
	string typewriter_font_name;
	///
	string popup_bold_font;
	///
	string popup_normal_font;
	///
	string font_norm;
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
	string popup_font_encoding;
	///
	unsigned int autosave;
	///
	string ascii_roff_command;
	///
	unsigned int ascii_linelen;
	/// Ispell command
#ifdef USE_PSPELL
	bool use_pspell;
#endif
	string isp_command;
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
	string isp_alt_lang;
	/// Alternate personal dictionary file for ispell
	string isp_pers_dict;
	/// Escape characters
	string isp_esc_chars;
	///
	bool use_kbmap;
	///
	string primary_kbmap;
	///
	string secondary_kbmap;
	///
	string lyxpipes;
	///
	string date_insert_format;
	///
	string language_package;
	///
	bool language_auto_begin;
	///
	bool language_auto_end;
	///
	string language_command_begin;
	///
	string language_command_end;
	///
	string language_command_local;
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
	/// Do we have to use a GUI?
	bool use_gui;
	///
	string default_language;
	///
	bool cursor_follows_scrollbar;
	///
	bool dialogs_iconify_with_main;
	///
	int label_init_length;
	///
	string display_graphics;
	///
	bool show_banner;
	///
	bool preview;
	///
	bool preview_hashed_labels;
	///
	float preview_scale_factor;
	
private:
	/// Is a bind file already (or currently) read?
	bool hasBindFile;
	///
	int ReadBindFile(string const & name);
};

///
extern LyXRC lyxrc;
///
extern LyXRC system_lyxrc;
#endif
