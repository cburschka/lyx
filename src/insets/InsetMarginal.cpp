/**
 * \file InsetMarginal.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMarginal.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "OutputParams.h"
#include "output_docbook.h"
#include "TocBackend.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

namespace lyx {


InsetMarginal::InsetMarginal(Buffer * buf)
	: InsetFootlike(buf)
{}


int InsetMarginal::plaintext(odocstringstream & os,
			     OutputParams const & runparams, size_t max_length) const
{
	os << '[' << buffer().B_("margin") << ":\n";
	InsetText::plaintext(os, runparams, max_length);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


void InsetMarginal::docbook(XMLStream & xs, OutputParams const & runparams) const
{
	// Implementation as per http://www.sagehill.net/docbookxsl/SideFloats.html
	// Unfortunately, only for XSL-FO output with the default style sheets, hence the role.
	xs << xml::StartTag("sidebar", "role=\"margin\"");
	xs << xml::CR();
	xs << XMLStream::ESCAPE_NONE << "<?dbfo float-type=\"margin.note\"?>";
	xs << xml::CR();
	InsetText::docbook(xs, runparams);
	xs << xml::EndTag("sidebar");
}


} // namespace lyx
