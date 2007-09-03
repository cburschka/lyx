/**
 * \file ControlPrint.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlPrint.h"

#include "frontend_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "PrinterParams.h"

#include "support/convert.h"
#include "support/FileFilterList.h"
#include "support/filetools.h"

using std::string;

namespace lyx {

using support::changeExtension;
using support::FileFilterList;

namespace frontend {


ControlPrint::ControlPrint(Dialog & parent)
	: Dialog::Controller(parent),
	  params_(0)
{}


bool ControlPrint::initialiseParams(std::string const &)
{
	/// get global printer parameters
	string const name =  changeExtension(kernel().buffer().fileName(),
					lyxrc.print_file_extension);
	params_.reset(new PrinterParams(PrinterParams::PRINTER,
					lyxrc.printer, name));

	dialog().setButtonsValid(true); // so that the user can press Ok
	return true;
}


void ControlPrint::clearParams()
{
	params_.reset();
}


PrinterParams & ControlPrint::params() const
{
	BOOST_ASSERT(params_.get());
	return *params_;
}


docstring const ControlPrint::browse(docstring const & in_name) const
{
	return browseRelFile(in_name, lyx::from_utf8(kernel().buffer().filePath()),
			     _("Print to file"),
			     FileFilterList(_("PostScript files (*.ps)")),
			     true);
}


/// print the current buffer
void ControlPrint::dispatchParams()
{
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
		command += convert<string>(pp.from_page);
		if (pp.to_page) {
			// we have a range "from-to"
			command += '-'
				+ convert<string>(pp.to_page);
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
			+ convert<string>(pp.count_copies)
			+ ' ';
	}

	if (pp.reverse_order) {
		command += lyxrc.print_reverse_flag + ' ';
	}

	if (!lyxrc.print_extra_options.empty()) {
		command += lyxrc.print_extra_options + ' ';
	}

	command += kernel().buffer().params().dvips_options();

	string const target = (pp.target == PrinterParams::PRINTER) ?
		"printer" : "file";

	string const target_name = (pp.target == PrinterParams::PRINTER) ?
		(pp.printer_name.empty() ? "default" : pp.printer_name) :
		pp.file_name;

	string const data = target + " \"" + target_name + "\" \"" + command + '"';
	kernel().dispatch(FuncRequest(getLfun(), data));
}

} // namespace frontend
} // namespace lyx
