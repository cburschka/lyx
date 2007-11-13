/**
 * \file output.cpp
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

#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lstrings.h"


namespace lyx {

using support::bformat;
using support::FileName;

using std::ofstream;
using std::string;

namespace {

template<typename OFStream>
bool doOpenFileWrite(OFStream & ofs, FileName const & fname)
{
	ofs.open(fname.toFilesystemEncoding().c_str());
	if (ofs)
		return true;
	docstring const file = fname.displayName(50);
	docstring text = bformat(_("Could not open the specified "
							 "document\n%1$s."), file);
	frontend::Alert::error(_("Could not open file"), text);
	return false;
}

}


bool openFileWrite(ofstream & ofs, FileName const & fname)
{
	return doOpenFileWrite(ofs, fname);
}


bool openFileWrite(odocfstream & ofs, FileName const & fname)
{
	return doOpenFileWrite(ofs, fname);
}


} // namespace lyx
