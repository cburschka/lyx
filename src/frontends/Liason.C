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
#include "converter.h"
#include "minibuffer.h"
#include "support/syscall.h"

using std::endl;

extern LyXRC lyxrc;

namespace Liason {

PrinterParams getPrinterParams(Buffer * buffer)
{
	return PrinterParams(PrinterParams::PRINTER,
			     lyxrc.printer,
			     ChangeExtension(buffer->fileName(),
					     lyxrc.print_file_extension));
}


bool printBuffer(Buffer * buffer, PrinterParams const & pp) 
{
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

	command += converters.dvips_options(buffer) + ' ';

	if (!Exporter::Export(buffer, "dvi", true))
		return false;

	// Push directory path.
	string path = OnlyPath(buffer->fileName());
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	Path p(path);

	// there are three cases here:
	// 1. we print to a file
	// 2. we print direct to a printer
	// 3. we print using a spool command (print to file first)
	Systemcalls one;
	int res = 0;
	string dviname = ChangeExtension(buffer->getLatexName(true), "dvi");
	switch (pp.target) {
	case PrinterParams::PRINTER:
		if (!lyxrc.print_spool_command.empty()) {
			// case 3
			string psname = ChangeExtension(dviname, ".ps");
			command += lyxrc.print_to_file
				+ QuoteName(psname) + ' ';
			command += QuoteName(dviname);
			string command2 = lyxrc.print_spool_command + ' ';
			if (!pp.printer_name.empty())
				command2 += lyxrc.print_spool_printerprefix
					+ pp.printer_name + ' ';
			command2 += QuoteName(psname);
			// First run dvips.
			// If successful, then spool command
			res = one.startscript(Systemcalls::System, command);
			if (res == 0)
				res = one.startscript(Systemcalls::SystemDontWait,
						      command2);
		} else
			// case 2
			res = one.startscript(Systemcalls::SystemDontWait, command);
		break;

	case PrinterParams::FILE:
		// case 1
		command += lyxrc.print_to_file
			+ QuoteName(MakeAbsPath(pp.file_name, path));
		command += ' ' + QuoteName(dviname);
		res = one.startscript(Systemcalls::SystemDontWait, command);
		break;
	}
	return res == 0;
}

void setMinibuffer(LyXView * lv, char const * msg)
{
	lv->getMiniBuffer()->Set(msg);
}

} // namespace Liason
