/**
 * \file dummy_impl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

/**
 * This file contains dummy implementation of some methods that are
 * needed byclasses used by tex2lyx. This allows to reduce the number
 * of classes we have to link against.
*/

// {[(

#include <config.h>

#include "Format.h"
#include "LaTeXFeatures.h"
#include "LyXRC.h"
#include "output_xhtml.h"

#include "support/Messages.h"

#include <iostream>

using namespace std;

namespace lyx {

//
// Dummy Alert support (needed by TextClass)
//


namespace frontend {
namespace Alert {
	void warning(docstring const & title, docstring const & message,
				 bool const &)
	{
		cerr << to_utf8(title) << "\n" << to_utf8(message) << endl;
	}
}
}


//
// Dummy TexRow support (needed by docstream)
//


void TexRow::newline()
{}


void TexRow::newlines(int)
{}


//
// Dummy LyXRC support
//

LyXRC lyxrc;

/** Note that some variables are not initialized correctly. Hopefully
 * they are not used in our code (currently valgrind does not complain).
 * Linking against the full LyXRC.cpp forces us to pull too much
 * stuff.
 */
LyXRC::LyXRC()
{}


//
// Dummy translation support (needed at many places)
//


Messages messages_;
Messages const & getMessages(string const &)
{
	return messages_;
}


Messages const & getGuiMessages()
{
	return messages_;
}


//
// Dummy formats support (needed by Lexer)
//

Formats formats;

bool Formats::isZippedFile(support::FileName const&) const
{
	return false;
}


//
// Dummy features support (needed by ModuleList)
//


bool LaTeXFeatures::isAvailable(string const &)
{
	return true;
}


string alignmentToCSS(LyXAlignment)
{
	return string();
}

//
// Dummy FontMetrics (needed by Length)
//


class FontMetrics {
	int em() const { return 0; };
};

class FontInfo;

FontMetrics const & theFontMetrics(FontInfo const &) {
	static FontMetrics dummy;
	return dummy;
}

//
// Keep the linker happy on Windows
//

void lyx_exit(int)
{}

}
