/**
 * \file ControlAboutlyx.cpp
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
#include "support/Package.h"

#include <fstream>
#include <sstream>

using std::ostream;
using std::ostringstream;
using std::string;


namespace lyx {

using support::FileName;
using support::fileSearch;
using support::makeDisplayPath;
using support::package;

namespace frontend {


ControlAboutlyx::ControlAboutlyx(Dialog & parent)
	: Dialog::Controller(parent)
{}


void ControlAboutlyx::getCredits(ostream & ss) const
{
	FileName const name = fileSearch(package().system_support().absFilename(), "CREDITS");

	bool found(!name.empty());

	if (found) {
		std::ifstream in(name.toFilesystemEncoding().c_str());

		ss << in.rdbuf();
		found = ss.good();
	}

	if (!found) {
		ss << to_utf8(_("ERROR: LyX wasn't able to read CREDITS file\n"))
		   << to_utf8(_("Please install correctly to estimate the great\n"))
		   << to_utf8(_("amount of work other people have done for the LyX project."));
	}
}


string const ControlAboutlyx::getCopyright() const
{
	return to_utf8(_("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-2008 LyX Team"));
}


string const ControlAboutlyx::getLicense() const
{
	return to_utf8(_("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version."));
}


string const ControlAboutlyx::getDisclaimer() const
{
	return to_utf8(_("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA."));
}


string const ControlAboutlyx::getVersion() const
{
	ostringstream ss;

	ss << to_utf8(_("LyX Version "))
	   << lyx_version
	   << " ("
	   << lyx_release_date
	   << ")\n"
	   << to_utf8(_("Library directory: "))
	   << to_utf8(makeDisplayPath(package().system_support().absFilename()))
	   << "\n"
	   << to_utf8(_("User directory: "))
	   << to_utf8(makeDisplayPath(package().user_support().absFilename()));

	return ss.str();
}

} // namespace frontend
} // namespace lyx
