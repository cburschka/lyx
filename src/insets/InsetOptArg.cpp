/**
 * \file InsetOptArg.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetOptArg.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"

using namespace std;

namespace lyx {


InsetOptArg::InsetOptArg(Buffer const & buf)
	: InsetCollapsable(buf)
{}


docstring InsetOptArg::editMessage() const
{
	return _("Opened Optional Argument Inset");
}


void InsetOptArg::write(ostream & os) const
{
	os << "OptArg" << "\n";
	InsetCollapsable::write(os);
}


int InsetOptArg::latex(odocstream &, OutputParams const &) const
{
	return 0;
}


int InsetOptArg::plaintext(odocstream &, OutputParams const &) const
{
	return 0; // do not output optional arguments
}


int InsetOptArg::docbook(odocstream &, OutputParams const &) const
{
	return 0;
}


int InsetOptArg::latexOptional(odocstream & os,
			       OutputParams const & runparams) const
{
	odocstringstream ss;
	int ret = InsetText::latex(ss, runparams);
	docstring str = ss.str();
	if (str.find(']') != docstring::npos)
		str = '{' + str + '}';
	os << '[' << str << ']';
	return ret;
}


} // namespace lyx
