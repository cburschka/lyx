/**
 * \file ControlAboutlyx.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlAboutlyx.h"
#include "Dialogs.h"
#include "frontends/LyXView.h"
#include "Lsstream.h"
#include "BufferView.h"
#include "gettext.h"
#include "support/filetools.h" // FileSearch
#include "version.h"

// needed for the browser
extern string system_lyxdir;
extern string user_lyxdir;

ControlAboutlyx::ControlAboutlyx(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{
	d_.showAboutlyx.connect(SigC::slot(this, &ControlAboutlyx::show));
}


stringstream & ControlAboutlyx::getCredits(stringstream & ss) const
{
	string const name = FileSearch(system_lyxdir, "CREDITS");

	bool found(!name.empty());

	if (found) {
		std::ifstream in(name.c_str());
		found = (in.get());

		if (found) {
			in.seekg(0, std::ios::beg); // rewind to the beginning

			ss << in.rdbuf();
			found = (ss.good());
		}
	}

	if (!found) {
		ss << _("ERROR: LyX wasn't able to read CREDITS file\n")
		   << _("Please install correctly to estimate the great\n")
		   << _("amount of work other people have done for the LyX project.");
	}

	return ss;
}

string const ControlAboutlyx::getCopyright() const
{
	return _("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-2001 LyX Team");
}

string const ControlAboutlyx::getLicense() const
{
	return _("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.");
}

string const ControlAboutlyx::getDisclaimer() const
{
	return _("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.");
}

string const ControlAboutlyx::getVersion() const
{
	stringstream ss;

	ss << _("LyX Version ")
	   << lyx_version
	   << _(" of ")
	   << lyx_release_date
	   << "\n"
	   << _("Library directory: ")
	   << MakeDisplayPath(system_lyxdir)
	   << "\n"
	   << _("User directory: ")
	   << MakeDisplayPath(user_lyxdir);

	return ss.str().c_str();
}
