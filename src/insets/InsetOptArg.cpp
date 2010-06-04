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


InsetOptArg::InsetOptArg(Buffer * buf)
	: InsetCollapsable(buf)
{}


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


docstring InsetOptArg::xhtml(XHTMLStream &, OutputParams const &) const
{
	return docstring();
}

int InsetOptArg::latexArgument(odocstream & os,
		OutputParams const & runparams, bool optional) const
{
	odocstringstream ss;
	int ret = InsetText::latex(ss, runparams);
	docstring str = ss.str();
	if (optional && str.find(']') != docstring::npos)
		str = '{' + str + '}';
	os << (optional ? '[' : '{') << str
	   << (optional ? ']' : '}');
	return ret;
}


} // namespace lyx
