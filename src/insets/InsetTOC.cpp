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
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "TocBackend.h"

#include "support/gettext.h"

#include <ostream>

using namespace std;

namespace lyx {


InsetTOC::InsetTOC(InsetCommandParams const & p)
	: InsetCommand(p, "toc")
{}


ParamInfo const & InsetTOC::findInfo(string const & /* cmdName */)
{
	static ParamInfo param_info_;
	if (param_info_.empty()) {
		param_info_.add("type", ParamInfo::LATEX_REQUIRED);
	}
	return param_info_;
}


docstring InsetTOC::screenLabel() const
{
	if (getCmdName() == "tableofcontents")
		return buffer().B_("Table of Contents");
	return _("Unknown TOC type");
}


int InsetTOC::plaintext(odocstream & os, OutputParams const &) const
{
	os << screenLabel() << "\n\n";
	buffer().tocBackend().writePlaintextTocList(getCmdName(), os);
	return PLAINTEXT_NEWLINE;
}


int InsetTOC::docbook(odocstream & os, OutputParams const &) const
{
	if (getCmdName() == "tableofcontents")
		os << "<toc></toc>";
	return 0;
}


} // namespace lyx
