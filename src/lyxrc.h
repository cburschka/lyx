// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXRC_H
#define LYXRC_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "ToolbarDefaults.h"
#include "bufferparams.h"

using std::map;

/// This contains the runtime configuration of LyX
class LyXRC {
public:
	LyXRC();
	///
	void setDefaults();
	///
	int read (string const & filename);
	///
	void write(string const & filename) const;
	///
        void print() const;
	///
	void output(ostream & os) const;
	/// Is a bind file already (or currently) read?
	bool hasBindFile;
	///
	int ReadBindFile(string const & name = "cua");
	///
	ToolbarDefaults toolbardefaults;
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
	/// program for running latex
	string latex_command;
	///
	string pdflatex_command;
	///
	string pdf_to_ps_command;
	///
	string dvi_to_ps_command;
        /// program for performing literate programming
        string literate_command;
        string literate_extension;
        string literate_error_filter;
        /// program for compiling
        string build_command;
        string build_error_filter;
	/// program for running relyx
	string relyx_command;
	/// postscript interpreter (in general "gs", if it is installed)
	string ps_command;
	/// program for viewing postscript output (default "ghostview -swap")
	string view_ps_command;
	/// program for viewing postscript pictures (default "ghostview")
	string view_pspic_command;
	/// program for viewing dvi output (default "xdvi")
	string view_dvi_command;
	/// option for telling the dvi viewer about the paper size
	string view_dvi_paper_option;
	/// program for viewing pdf output (default "xpdf")
	string view_pdf_command;
        /// default paper size for local xdvi/dvips/ghostview/whatever
        BufferParams::PAPER_SIZE default_papersize;
	/// command to run chktex incl. options
	string chktex_command;
	/// command to run an html converter incl. options
	string html_command;
	///
	string sgml_extra_options;
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
	bool check_lastfiles;
	/// filename for lastfiles file
	string lastfiles;
	/// maximal number of lastfiles
	unsigned int num_lastfiles;
	/// shall a backup file be created
	bool make_backup;
	/// Zoom factor for screen fonts
	unsigned int zoom;
	/// Screen font sizes in points for each font size
	float font_sizes[10];
	/// Allow the use of scalable fonts? Default is yes.
	bool use_scalable_fonts;
	/// DPI of monitor
	float dpi;
	///
	string fontenc;
	///
	string roman_font_name;
	///
	string sans_font_name;
	///
	string typewriter_font_name;
	///
	string menu_font_name;
	///
	string popup_font_name;
	///
	string font_norm;
	///
	string font_norm_menu;
	///
	unsigned int autosave;
	///
	string fax_command;
	///
	string phone_book;
	///
	string fax_program;
	///
	string ascii_roff_command;
	///
	unsigned int ascii_linelen;
	/// Ispell command
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
	/// Ask for confirmation of exit when there are unsaved documents?
	bool exit_confirmation;
	/// Should we display short-cut information in the minibuffer?
	bool display_shortcuts;
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
	bool rtl_support;
	///
	string auto_mathmode;
	///
	bool show_banner;
	///
	typedef map<string, int> Bindings;
	///
	Bindings bindings;
private:
	///
	void defaultKeyBindings();
};

extern LyXRC lyxrc;

#endif
