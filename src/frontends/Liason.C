/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Liason.h"
#include "LyXView.h"
#include "BufferView.h"
#include "buffer.h"
#include "lyxrc.h"
#include "PrinterParams.h"
#include "lyx_gui_misc.h"
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/path.h"
#include "exporter.h"
#include "minibuffer.h"

extern LyXRC lyxrc;
extern bool RunScript(Buffer * buffer, bool wait, string const & command,
		      string const & orgname = string(), bool need_shell=true);


#ifdef CXX_WORKING_NAMESPACES
namespace Liason 
{
#endif

PrinterParams getPrinterParams(Buffer * buffer)
{
	return PrinterParams(PrinterParams::PRINTER,
			     lyxrc.printer,
			     ChangeExtension(buffer->fileName(),
					     lyxrc.print_file_extension));
}


bool printBuffer(Buffer * buffer, PrinterParams const & pp) 
{
	bool result(false);
	string command(lyxrc.print_command + ' ');
	
	if (pp.target == PrinterParams::PRINTER
	    && lyxrc.print_adapt_output  // dvips wants a printer name
	    && !pp.printer_name.empty()) {// printer name given
		command += lyxrc.print_to_printer
			+ pp.printer_name
			+ ' ';
	}

	switch (pp.which_pages) {
	case PrinterParams::EVEN:
		command += lyxrc.print_evenpage_flag + ' ';
		break;

	case PrinterParams::ODD:
		command += lyxrc.print_oddpage_flag + ' ';
		break;
    
	default:
		// only option left is print all of them
		break;
	}

	if (!pp.from_page.empty()) {
		command += lyxrc.print_pagerange_flag + ' ';
		command += pp.from_page;
		if (pp.to_page) {
				// we have a range "from-to"
			command += '-';
			command += tostr(pp.to_page);
		}
		command += ' ';
	}

	if (pp.reverse_order) {
		command += lyxrc.print_reverse_flag + ' ';
	}

	BufferParams params(buffer->params);
	if (params.orientation
	    == BufferParams::ORIENTATION_LANDSCAPE) {
		command += lyxrc.print_landscape_flag + ' ';
	}

	if (1 < pp.count_copies) {
		if (pp.unsorted_copies) {
			command += lyxrc.print_copies_flag;
		} else {
			command += lyxrc.print_collcopies_flag;
		}
		command += ' ';
		command += tostr(pp.count_copies);
		command += ' ';
	}

	if (!lyxrc.print_extra_options.empty()) {
		command += lyxrc.print_extra_options + ' ';
	}

	char real_papersize = params.papersize;
	if (real_papersize == BufferParams::PAPER_DEFAULT) {
		real_papersize = lyxrc.default_papersize;
	}

	if (params.use_geometry
	    && params.papersize2 == BufferParams::VM_PAPER_CUSTOM
	    && !lyxrc.print_paper_dimension_flag.empty()
	    && !params.paperwidth.empty()
	    && !params.paperheight.empty()) {
		// using a custom papersize
		command += lyxrc.print_paper_dimension_flag + ' ';
		command += params.paperwidth + ',';
		command += params.paperheight + ' ';
	} else if (!lyxrc.print_paper_flag.empty()
		   && (real_papersize != BufferParams::PAPER_USLETTER
		       || params.orientation 
		       == BufferParams::ORIENTATION_PORTRAIT)) {
		// There's a problem with US Letter + landscape
		string paper;
		switch (real_papersize) {
		case BufferParams::PAPER_USLETTER:
			paper = "letter";
			break;
		case BufferParams::PAPER_A3PAPER:
			paper = "a3";
			break;
		case BufferParams::PAPER_A4PAPER:
			paper = "a4";
			break;
		case BufferParams::PAPER_A5PAPER:
			paper = "a5";
			break;
		case BufferParams::PAPER_B5PAPER:
			paper = "b5";
			break;
		case BufferParams::PAPER_EXECUTIVEPAPER:
			paper = "foolscap";
			break;
		case BufferParams::PAPER_LEGALPAPER:
			paper = "legal";
			break;
		default: /* If nothing else fits, keep empty value */
			break;
		}
		if (!paper.empty()) {
			command += lyxrc.print_paper_flag + ' ';
			command += paper + ' ';
		}
	}

	// Push directory path if necessary.
	// PS file should go where the source file is unless it's a
	// read-only directory in which case we write it to tmpdir.
	// All temporary files go in tmpdir (like spool files).
	string path = OnlyPath(buffer->fileName());
	if ((pp.target != PrinterParams::FILE
	     && lyxrc.use_tempdir)
	    || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	Path p(path);

#ifndef NEW_EXPORT
	// there are three cases here:
	// 1. we print to a file
	// 2. we print direct to a printer
	// 3. we print using a spool command (print to file first)
	switch (pp.target) {
	case PrinterParams::PRINTER:
		if (!lyxrc.print_spool_command.empty()) {
				// case 3
			command += lyxrc.print_to_file
				+ QuoteName(pp.file_name);
			string command2 = lyxrc.print_spool_command
				+ ' ';
			if (!pp.printer_name.empty()) {
				command2 += lyxrc.print_spool_printerprefix
					+ pp.printer_name;
			}
				// First run dvips.
				// If successful, then spool command
			if (result = RunScript(buffer, true, command)) {
				result = RunScript(buffer, false,
						   command2,
						   QuoteName(pp.file_name));
			}
		} else {
				// case 2
			result = RunScript(buffer, false, command);
		}
		break;

	case PrinterParams::FILE:
		// case 1
		command += lyxrc.print_to_file
			+ QuoteName(MakeAbsPath(pp.file_name, path));
		result = RunScript(buffer, false, command);
		break;
	}
#endif
	return result;
}

void setMinibuffer(LyXView * lv, char const * msg)
{
	lv->getMiniBuffer()->Set(msg);
}

#ifdef CXX_WORKING_NAMESPACES
}
#endif

