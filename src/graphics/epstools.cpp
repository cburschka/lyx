/**
 * \file epstools.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * parts Copyright 1985, 1990, 1993 Free Software Foundation, Inc.
 *
 * \author Ivan Schreter
 * \author Dirk Niggemann
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 *
 * Utilities for manipulation of Encapsulated Postscript files
 */

#include <config.h>

#include "graphics/epstools.h"

#include "Format.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/regex.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace graphics {


string const readBB_from_PSFile(FileName const & file)
{
	// in a (e)ps-file it's an entry like %%BoundingBox:23 45 321 345
	// It seems that every command in the header has an own line,
	// getline() should work for all files.
	// On the other hand some plot programs write the bb at the
	// end of the file. Than we have in the header:
	// %%BoundingBox: (atend)
	// In this case we must check the end.
	bool const zipped = formats.isZippedFile(file);
	FileName const file_ = zipped ? unzipFile(file) : file;
	string const format = formats.getFormatFromFile(file_);

	if (!Formats::isPostScriptFileFormat(format)) {
		LYXERR(Debug::GRAPHICS, "[readBB_from_PSFile] no(e)ps-format");
		if (zipped)
			file_.removeFile();
		return string();
	}

	static lyx::regex bbox_re("^%%BoundingBox:\\s*([-]*[[:digit:]]+)"
		"\\s+([-]*[[:digit:]]+)\\s+([-]*[[:digit:]]+)\\s+([-]*[[:digit:]]+)");
	ifstream is(file_.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		getline(is,s);
		lyx::smatch what;
		if (regex_match(s, what, bbox_re)) {
			// Our callers expect the tokens in the string
			// separated by single spaces.
			// FIXME: change return type from string to something
			// sensible
			ostringstream os;
			os << what.str(1) << ' ' << what.str(2) << ' '
			   << what.str(3) << ' ' << what.str(4);
			string const bb = os.str();
			LYXERR(Debug::GRAPHICS, "[readBB_from_PSFile] " << bb);
			if (zipped)
				file_.removeFile();
			return bb;
		}
	}
	LYXERR(Debug::GRAPHICS, "[readBB_from_PSFile] no bb found");
	if (zipped)
		file_.removeFile();
	return string();
}


} // namespace graphics
} // namespace lyx
