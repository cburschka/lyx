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

#include "LaTeXFeatures.h"
#include "LyXRC.h"
#include "output_xhtml.h"
#include "xml.h"

#include "support/Messages.h"

#include <iostream>

using namespace std;

namespace lyx {

//
// Dummy Alert support (needed by TextClass)
//


namespace frontend {
namespace Alert {
	void warning(docstring const & title, docstring const & message, bool)
	{
		cerr << to_utf8(title) << "\n" << to_utf8(message) << endl;
	}
} // namespace Alert
} // namespace frontend


//
// Required global variables
//

bool verbose = false;
LyXRC lyxrc;


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
// Keep the linker happy on Windows
//

void lyx_exit(int)
{}

namespace xml {
docstring StartTag::writeTag() const { return docstring(); }
docstring StartTag::writeEndTag() const { return docstring(); }
bool StartTag::operator==(FontTag const & rhs) const { return rhs == *this; }
bool FontTag::operator==(StartTag const & tag) const { FontTag const * const ftag = tag.asFontTag(); if (!ftag) return false; return (font_type_ == ftag->font_type_); }
}

} // namespace lyx
