/**
 * \file InsetTOC.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetTOC.h"

#include "Buffer.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TocBackend.h"

#include "support/std_ostream.h"


namespace lyx {

using std::string;
using std::ostream;


InsetTOC::InsetTOC(InsetCommandParams const & p)
	: InsetCommand(p, "toc")
{}


Inset * InsetTOC::clone() const
{
	return new InsetTOC(*this);
}


docstring const InsetTOC::getScreenLabel(Buffer const & buf) const
{
	if (getCmdName() == "tableofcontents")
		return buf.B_("Table of Contents");
	return _("Unknown TOC type");
}


int InsetTOC::plaintext(Buffer const & buffer, odocstream & os,
			OutputParams const &) const
{
	os << getScreenLabel(buffer) << "\n\n";

	buffer.tocBackend().writePlaintextTocList(getCmdName(), os);

	return PLAINTEXT_NEWLINE;
}


int InsetTOC::docbook(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}


} // namespace lyx
