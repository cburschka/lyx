/**
 * \file ControlSendto.C
 * Copyright 2002 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlSendto.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"
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
{
	d_.showSendto.connect(SigC::slot(this, &ControlSendto::show));
}


vector<Format const *> const ControlSendto::allFormats() const
{
	// What formats can we output natively?
	vector<string> exports;
	exports.push_back("lyx");
	exports.push_back("text");

	if (lv_.buffer()->isLatex())
		exports.push_back("latex");
	if (lv_.buffer()->isLinuxDoc())
		exports.push_back("linuxdoc");
	if (lv_.buffer()->isDocBook())
		exports.push_back("docbook");
	if (lv_.buffer()->isLiterate())
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
			if (converters.isReachable(*ex_it, fo_it->name())) {
				to.push_back(fo_it);
			}
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
	command_ = strip(frontStrip(cmd));
}


void ControlSendto::apply()
{
	if (!lv_.view()->available())
		return;
   
	view().apply();

	if (command_.empty() || !format_)
		return;

	// The name of the file created by the conversion process
	string filename;

	// Output to filename
	if (format_->name() == "lyx") {
		filename = ChangeExtension(lv_.buffer()->getLatexName(false),
					   format_->extension());
		if (!lv_.buffer()->tmppath.empty())
			filename = AddName(lv_.buffer()->tmppath, filename);

		lv_.buffer()->writeFile(filename, true);

	} else {
		Exporter::Export(lv_.buffer(), format_->name(), true, filename);
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

