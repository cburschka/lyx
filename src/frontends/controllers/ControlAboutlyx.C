/**
 * \file ControlAboutlyx.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlAboutlyx.h"
#include "gettext.h"
#include "version.h"

#include "support/filetools.h" // FileSearch
#include "support/package.h"

#include <fstream>
#include <sstream>

using std::ostream;
using std::ostringstream;
using std::string;


namespace lyx {

using support::fileSearch;
using support::makeDisplayPath;
using support::package;

namespace frontend {


ControlAboutlyx::ControlAboutlyx(Dialog & parent)
	: Dialog::Controller(parent)
{}


void ControlAboutlyx::getCredits(ostream & ss) const
{
	string const name = fileSearch(package().system_support(), "CREDITS");

	bool found(!name.empty());

	if (found) {
		std::ifstream in(name.c_str());

		ss << in.rdbuf();
		found = ss.good();
	}

	if (!found) {
		ss << lyx::to_utf8(_("ERROR: LyX wasn't able to read CREDITS file\n"))
		   << lyx::to_utf8(_("Please install correctly to estimate the great\n"))
		   << lyx::to_utf8(_("amount of work other people have done for the LyX project."));
	}
}


string const ControlAboutlyx::getCopyright() const
{
	return lyx::to_utf8(_("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-2001 LyX Team"));
}


string const ControlAboutlyx::getLicense() const
{
	return lyx::to_utf8(_("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version."));
}


string const ControlAboutlyx::getDisclaimer() const
{
	return lyx::to_utf8(_("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA."));
}


string const ControlAboutlyx::getVersion() const
{
	ostringstream ss;

	ss << lyx::to_utf8(_("LyX Version "))
	   << lyx_version
	   << " ("
	   << lyx_release_date
	   << ")\n"
	   << lyx::to_utf8(_("Library directory: "))
	   << makeDisplayPath(package().system_support())
	   << "\n"
	   << lyx::to_utf8(_("User directory: "))
	   << makeDisplayPath(package().user_support());

	return ss.str();
}

} // namespace frontend
} // namespace lyx
