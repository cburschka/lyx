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
	// Well, we can start with "lyx"
	vector<Format const *> lyx_formats;
	lyx_formats.push_back(formats.getFormat("lyx"));

	// lyx can export to latex, what formats can be reached from latex?
	Formats::const_iterator it  = formats.begin();
	Formats::const_iterator end = formats.end();
	vector<Format const *>::const_iterator lbegin = lyx_formats.begin();
	for (; it != end; ++it) {
		if (converters.isReachable("latex", it->name())) {
			vector<Format const *>::const_iterator lend =
				lyx_formats.end();
			
			if (std::find(lbegin, lend, it) == lend)
				lyx_formats.push_back(it);
		}
	}

	return lyx_formats;
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
	string filename = lv_.buffer()->getLatexName(false);
	if (!lv_.buffer()->tmppath.empty())
		filename = AddName(lv_.buffer()->tmppath, filename);
	filename = ChangeExtension(filename, format_->extension());

	// Output to filename
	if (format_->name() == "lyx") {
		lv_.buffer()->writeFile(filename, true);

	} else {
		Exporter::Export(lv_.buffer(), format_->name(), true,
				 filename);
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

