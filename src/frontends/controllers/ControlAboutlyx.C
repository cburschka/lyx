/**
 * \file ControlAboutlyx.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlAboutlyx.h"
#include "gettext.h"
#include "version.h"
#include "Lsstream.h"

#include "support/LOstream.h"
#include "support/filetools.h" // FileSearch

#include <fstream>

using std::ostream;

// needed for the browser
extern string system_lyxdir;
extern string user_lyxdir;


ControlAboutlyx::ControlAboutlyx(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{}


void ControlAboutlyx::getCredits(ostream & ss) const
{
	string const name = FileSearch(system_lyxdir, "CREDITS");

	bool found(!name.empty());

	if (found) {
		std::ifstream in(name.c_str());

		ss << in.rdbuf();
		found = ss.good();
	}

	if (!found) {
		ss << _("ERROR: LyX wasn't able to read CREDITS file\n")
		   << _("Please install correctly to estimate the great\n")
		   << _("amount of work other people have done for the LyX project.");
	}
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
	ostringstream ss;

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
