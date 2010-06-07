/**
 * \file InsetArgument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetArgument.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"

using namespace std;

namespace lyx {


InsetArgument::InsetArgument(Buffer * buf)
	: InsetCollapsable(buf)
{}


void InsetArgument::write(ostream & os) const
{
	os << "Argument" << "\n";
	InsetCollapsable::write(os);
}


int InsetArgument::latex(odocstream &, OutputParams const &) const
{
	return 0;
}


int InsetArgument::plaintext(odocstream &, OutputParams const &) const
{
	return 0; // do not output optional arguments
}


int InsetArgument::docbook(odocstream &, OutputParams const &) const
{
	return 0;
}


docstring InsetArgument::xhtml(XHTMLStream &, OutputParams const &) const
{
	return docstring();
}

int InsetArgument::latexArgument(odocstream & os,
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
