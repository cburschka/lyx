/**
 * \file lyxfont.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "lyxfont.h"
#include "lyxlex.h"
#include "support/lstrings.h"

using namespace lyx::support;


LyXFont & LyXFont::lyxRead(LyXLex & lex)
{
	bool error = false;
	bool finished = false;
	while (!finished && lex.isOK() && !error) {
		lex.next();
		string const tok = ascii_lowercase(lex.getString());

		if (tok.empty()) {
			continue;
		} else if (tok == "endfont") {
			finished = true;
		} else if (tok == "family") {
			lex.next();
		} else if (tok == "series") {
			lex.next();
		} else if (tok == "shape") {
			lex.next();
		} else if (tok == "size") {
			lex.next();
		} else if (tok == "misc") {
			lex.next();
		} else if (tok == "color") {
			lex.next();
		} else {
			lex.printError("Unknown tag `$$Token'");
			error = true;
		}
	}
	return *this;
}
