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

#include "dispatchresult.h"
#include "funcrequest.h"
#include "gettext.h"
#include "metricsinfo.h"
#include "toc.h"

#include "support/std_ostream.h"

using std::string;
using std::ostream;



InsetTOC::InsetTOC(InsetCommandParams const & p)
	: InsetCommand(p, "toc")
{}


std::auto_ptr<InsetBase> InsetTOC::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetTOC(*this));
}


string const InsetTOC::getScreenLabel(Buffer const &) const
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


int InsetTOC::plaintext(Buffer const & buffer, ostream & os,
		    OutputParams const &) const
{
	os << getScreenLabel(buffer) << "\n\n";

	lyx::toc::asciiTocList(lyx::toc::getType(getCmdName()), buffer, os);

	os << "\n";
	return 0;
}


int InsetTOC::linuxdoc(Buffer const &, ostream & os,
		       OutputParams const &) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc>";
	return 0;
}


int InsetTOC::docbook(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}
