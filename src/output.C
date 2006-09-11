/**
 * \file output.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output.h"

#include "gettext.h"

#include "frontends/Alert.h"

#include "support/filetools.h"

#include <fstream>

using lyx::support::bformat;
using lyx::support::makeDisplayPath;

using lyx::docstring;

using std::ofstream;
using std::string;

bool openFileWrite(ofstream & ofs, string const & fname)
{
	ofs.open(fname.c_str());
	if (!ofs) {
		docstring const file = makeDisplayPath(fname, 50);
		docstring text = bformat(_("Could not open the specified "
						     "document\n%1$s."), file);
		Alert::error(_("Could not open file"), text);
		return false;
	}
	return true;
}
