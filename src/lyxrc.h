// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 1996 Matthias Ettrich
 *           and the LyX Team.
 *
 *======================================================*/

#ifndef _LYXRC_H
#define _LYXRC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "toolbar.h"

/// This contains the runtime configuration of LyX
class LyXRC {
public:
	///
	LyXRC();
	///
	~LyXRC();
	///
	int Read (LString const & filename);
	///
        void Print();
	/// Is a bind file already (or currently) read?
	bool hasBindFile;
	///
	int ReadBindFile(LString name = "cua");
	///
	Toolbar toolbar;
	///
	LString printer;
	///
	LString print_command;
	///
	LString print_evenpage_flag;
	///
	LString print_oddpage_flag;
	///
	LString print_pagerange_flag;
	///
	LString print_copies_flag;
	///
	LString print_collcopies_flag;
	///
	LString print_reverse_flag;
	///
	LString print_landscape_flag;
	///
	LString print_to_printer;
	///
	bool print_adapt_output;
	///
	LString print_to_file;
	///
	LString print_file_extension;
	///
	LString print_extra_options;
	///
	LString print_spool_command;
        ///
	LString print_spool_printerprefix;
	///
	LString print_paper_flag;
	///
	LString print_paper_dimension_flag;
	///
        LString custom_export_command;
	///
	LString custom_export_format;
	/// program for running latex
	LString latex_command;
        /// program for performing literate programming
        LString literate_command;
        LString literate_extension;
        LString literate_error_filter;
        /// program for compiling
        LString build_command;
        LString build_error_filter;
	/// program for running relyx
	LString relyx_command;
	/// postscript interpreter (in general "gs", if it is installed)
	LString ps_command;
	/// program for viewing postscript output (default "ghostview -swap")
	LString view_ps_command;
	/// program for viewing postscript pictures (default "ghostview")
	LString view_pspic_command;
	/// program for viewing dvi output (default "xdvi")
	LString view_dvi_command;
        /// default paper size for local xdvi/dvips/ghostview/whatever
        LYX_PAPER_SIZE default_papersize;
	/// command to run chktex incl. options
	LString chktex_command;
	///
	LString sgml_extra_options;
	///
	LString document_path;
	///
	LString template_path;
	///
	LString tempdir_path;
	///
	bool use_tempdir;
	///
	bool auto_region_delete;
	/// flag telling whether lastfiles should be checked for existance
	bool check_lastfiles;
	/// filename for lastfiles file
	LString lastfiles;
	/// maximal number of lastfiles
	unsigned int num_lastfiles;
	/// Zoom factor for screen fonts
	unsigned int zoom;
	/// Screen font sizes in points for each font size
	float font_sizes[10];
	/// Allow the use of scalable fonts? Default is yes.
	bool use_scalable_fonts;
	/// DPI of monitor
	float dpi;
	///
	LString fontenc;
	///
	LString roman_font_name;
	///
	LString sans_font_name;
	///
	LString typewriter_font_name;
	///
	LString menu_font_name;
	///
	LString popup_font_name;
	///
	LString font_norm;
	///
	unsigned int autosave;
	///
	LString fax_command;
	///
	LString phone_book;
	///
	LString fax_program;
	///
	LString ascii_roff_command;
	///
	unsigned int ascii_linelen;
	/// Ispell command
	LString isp_command;
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
	LString isp_alt_lang;
	/// Alternate personal dictionary file for ispell
	LString isp_pers_dict;
	/// Escape characters
	LString isp_esc_chars;
	///
	bool use_kbmap;
	/// Ask for confirmation of exit when there are unsaved documents?
	bool exit_confirmation;
	/// Should we display short-cut information in the minibuffer?
	bool display_shortcuts;
	///
	LString primary_kbmap;
	///
	LString secondary_kbmap;
	///
	LString lyxpipes;

private:
	///
	void defaultKeyBindings();
};

#endif
