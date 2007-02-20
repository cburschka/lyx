/**
 * \file insettoc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettoc.h"

#include "buffer.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "metricsinfo.h"
#include "outputparams.h"
#include "TocBackend.h"

#include "support/std_ostream.h"


namespace lyx {

using std::string;
using std::ostream;


InsetTOC::InsetTOC(InsetCommandParams const & p)
	: InsetCommand(p, "toc")
{}


std::auto_ptr<InsetBase> InsetTOC::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetTOC(*this));
}


docstring const InsetTOC::getScreenLabel(Buffer const &) const
{
	if (getCmdName() == "tableofcontents")
		return _("Table of Contents");
	return _("Unknown toc list");
}


InsetBase::Code InsetTOC::lyxCode() const
{
	if (getCmdName() == "tableofcontents")
		return InsetBase::TOC_CODE;
	return InsetBase::NO_CODE;
}


int InsetTOC::plaintext(Buffer const & buffer, odocstream & os,
                        OutputParams const & runparams) const
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
