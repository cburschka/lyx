/**
 * \file ControlSendto.C
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

#include "ControlSendto.h"
#include "ViewBase.h"
#include "buffer.h"
#include "converter.h"
#include "exporter.h"
#include "gettext.h"
#include "lyxrc.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/systemcall.h"

using std::vector;


ControlSendto::ControlSendto(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  format_(0),
	  command_(lyxrc.custom_export_command)
{}


vector<Format const *> const ControlSendto::allFormats() const
{
	// What formats can we output natively?
	vector<string> exports;
	exports.push_back("lyx");
	exports.push_back("text");

	if (buffer()->isLatex())
		exports.push_back("latex");
	if (buffer()->isLinuxDoc())
		exports.push_back("linuxdoc");
	if (buffer()->isDocBook())
		exports.push_back("docbook");
	if (buffer()->isLiterate())
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
			    converters.isReachable(*ex_it, name))
				to.push_back(&(*fo_it));
		}
	}

	// Remove repeated formats.
	std::sort(to.begin(), to.end());

	vector<Format const *>::iterator to_begin = to.begin();
	vector<Format const *>::iterator to_end   = to.end();
	vector<Format const *>::iterator to_it =
		std::unique(to_begin, to_end);
	if (to_it != to_end)
		to.erase(to_it, to_end);

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


void ControlSendto::apply()
{
	if (!bufferIsAvailable())
		return;

	view().apply();

	if (command_.empty() || !format_)
		return;

	// The name of the file created by the conversion process
	string filename;

	// Output to filename
	if (format_->name() == "lyx") {
		filename = ChangeExtension(buffer()->getLatexName(false),
					   format_->extension());
		if (!buffer()->tmppath.empty())
			filename = AddName(buffer()->tmppath, filename);

		buffer()->writeFile(filename);

	} else {
		Exporter::Export(buffer(), format_->name(), true, filename);
	}

	// Substitute $$FName for filename
	string command = command_;
	if (!contains(command, "$$FName"))
		command = "( " + command + " ) < $$FName";
	command = subst(command, "$$FName", filename);

	// Execute the command in the background
	Systemcall call;
	call.startscript(Systemcall::DontWait, command);
}
