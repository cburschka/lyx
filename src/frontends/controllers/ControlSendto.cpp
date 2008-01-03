/**
 * \file ControlSendto.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlSendto.h"

#include "Buffer.h"
#include "Converter.h"
#include "Format.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include "support/filetools.h"
#include "support/lstrings.h"

using std::vector;
using std::string;

namespace lyx {

using support::trim;

namespace frontend {


ControlSendto::ControlSendto(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlSendto::initialiseParams(std::string const &)
{
	format_ = 0;
	command_ = lyxrc.custom_export_command;
	return true;
}


void ControlSendto::dispatchParams()
{
	if (command_.empty() || !format_ || format_->name().empty())
		return;

	string const data = format_->name() + " " + command_;
	kernel().dispatch(FuncRequest(getLfun(), data));
}


vector<Format const *> const ControlSendto::allFormats() const
{
	// What formats can we output natively?
	vector<string> exports;
	exports.push_back("lyx");
	exports.push_back("text");

	Buffer const & buffer = kernel().buffer();

	if (buffer.isLatex()) {
		exports.push_back("latex");
		exports.push_back("pdflatex");
	}
	else if (buffer.isDocBook())
		exports.push_back("docbook");
	else if (buffer.isLiterate())
		exports.push_back("literate");

	// Loop over these native formats and ascertain what formats we
	// can convert to
	vector<Format const *> to;

	vector<string>::const_iterator ex_it  = exports.begin();
	vector<string>::const_iterator ex_end = exports.end();
	for (; ex_it != ex_end; ++ex_it) {
		// Start off with the native export format.
		// "formats" is LyX's list of recognised formats
		to.push_back(formats.getFormat(*ex_it));

		Formats::const_iterator fo_it  = formats.begin();
		Formats::const_iterator fo_end = formats.end();
		for (; fo_it != fo_end; ++fo_it) {
			// we need to hide the default graphic export formats
			// from the external menu, because we need them only
			// for the internal lyx-view and external latex run
			string const name = fo_it->name();
			if (name != "eps" && name != "xpm" && name != "png" &&
			    theConverters().isReachable(*ex_it, name))
				to.push_back(&(*fo_it));
		}
	}

	// Remove repeated formats.
	std::sort(to.begin(), to.end());
	to.erase(std::unique(to.begin(), to.end()), to.end());

	return to;
}


void ControlSendto::setFormat(Format const * fmt)
{
	format_ = fmt;
}


void ControlSendto::setCommand(string const & cmd)
{
	command_ = trim(cmd);
}

} // namespace frontend
} // namespace lyx
