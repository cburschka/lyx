/**
 * \file ControlPrint.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlPrint.h"

#include "ViewBase.h"
#include "ButtonControllerBase.h"

#include "buffer.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "PrinterParams.h"
#include "exporter.h"
#include "converter.h"

#include "frontends/Alert.h"

#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/path.h"
#include "support/systemcall.h"

#include "debug.h" // for lyxerr

using std::endl;

ControlPrint::ControlPrint(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  params_(0)
{}


PrinterParams & ControlPrint::params() const
{
	lyx::Assert(params_);
	return *params_;
}


void ControlPrint::setParams()
{
	if (params_) delete params_;

	/// get global printer parameters
	string const name =  ChangeExtension(buffer()->fileName(),
					lyxrc.print_file_extension);
	params_ = new PrinterParams (PrinterParams::PRINTER,
					lyxrc.printer, name);

	bc().valid(); // so that the user can press Ok
}


void ControlPrint::clearParams()
{
	if (params_) {
		delete params_;
		params_ = 0;
	}
}


string const ControlPrint::Browse(string const & in_name)
{
	string const title = _("Print to file");
	string const pattern = "*.ps";

	// Show the file browser dialog
	return browseRelFile(&lv_, in_name, buffer()->filePath(),
			     title, pattern);
}


/// print the current buffer
void ControlPrint::apply()
{
	if (!bufferIsAvailable())
		return;

	view().apply();

	PrinterParams const pp = params();
	string command(lyxrc.print_command + ' ');

	if (pp.target == PrinterParams::PRINTER
	    && lyxrc.print_adapt_output  // dvips wants a printer name
	    && !pp.printer_name.empty()) {// printer name given
		command += lyxrc.print_to_printer
			+ pp.printer_name
			+ ' ';
	}

	if (!pp.all_pages && pp.from_page) {
		command += lyxrc.print_pagerange_flag + ' ';
		command += tostr(pp.from_page);
		if (pp.to_page) {
			// we have a range "from-to"
			command += '-'
				+ tostr(pp.to_page);
		}
		command += ' ';
	}

	// If both are, or both are not selected, then skip the odd/even printing
	if (pp.odd_pages != pp.even_pages) {
		if (pp.odd_pages) {
			command += lyxrc.print_oddpage_flag + ' ';
		} else if (pp.even_pages) {
			command += lyxrc.print_evenpage_flag + ' ';
		}
	}

	if (pp.count_copies > 1) {
		if (pp.sorted_copies) {
			command += lyxrc.print_collcopies_flag;
		} else {
			command += lyxrc.print_copies_flag;
		}
		command += ' '
			+ tostr(pp.count_copies)
			+ ' ';
	}
	
	if (pp.reverse_order) {
		command += lyxrc.print_reverse_flag + ' ';
	}

	if (!lyxrc.print_extra_options.empty()) {
		command += lyxrc.print_extra_options + ' ';
	}

	command += converters.dvips_options(buffer()) + ' ';

	if (!Exporter::Export(buffer(), "dvi", true)) {
		Alert::alert(_("Error:"),
			   _("Unable to print"),
			   _("Check that your parameters are correct"));
		return;
	}

	// Push directory path.
	string path = buffer()->filePath();
	if (lyxrc.use_tempdir || !IsDirWriteable(path)) {
		path = buffer()->tmppath;
	}
	Path p(path);

	// there are three cases here:
	// 1. we print to a file
	// 2. we print directly to a printer
	// 3. we print using a spool command (print to file first)
	Systemcall one;
	int res = 0;
	string const dviname = ChangeExtension(buffer()->getLatexName(true), "dvi");
	switch (pp.target) {
	case PrinterParams::PRINTER:
		if (!lyxrc.print_spool_command.empty()) {
			// case 3: print using a spool
			string const psname = ChangeExtension(dviname, ".ps");
			command += lyxrc.print_to_file
				+ QuoteName(psname)
				+ ' '
				+ QuoteName(dviname);

			string command2 = lyxrc.print_spool_command + ' ';
			if (!pp.printer_name.empty()) {
				command2 += lyxrc.print_spool_printerprefix
					+ pp.printer_name
					+ ' ';
			}
			command2 += QuoteName(psname);
			// First run dvips.
			// If successful, then spool command
			res = one.startscript(Systemcall::Wait, command);
			if (res == 0)
				res = one.startscript(Systemcall::DontWait,
						      command2);
		} else {
			// case 2: print directly to a printer
			res = one.startscript(Systemcall::DontWait,
					      command + QuoteName(dviname));
		}
		break;

	case PrinterParams::FILE:
		// case 1: print to a file
		command += lyxrc.print_to_file
			+ QuoteName(MakeAbsPath(pp.file_name, path))
			+ ' ' 
			+ QuoteName(dviname);
		res = one.startscript(Systemcall::DontWait, command);
		break;
	}

	lyxerr[Debug::LATEX] << "ControlPrint::apply(): print command = \"" << command << "\"" << endl;

	if (res != 0) {
		Alert::alert(_("Error:"),
			   _("Unable to print"),
			   _("Check that your parameters are correct"));
	}		
}
