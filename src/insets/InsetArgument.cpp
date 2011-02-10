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


void InsetArgument::latex(otexstream &, OutputParams const &) const
{}


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

void InsetArgument::latexArgument(otexstream & os,
		OutputParams const & runparams, bool optional) const
{
	TexRow texrow;
	odocstringstream ss;
	otexstream ots(ss, texrow);
	InsetText::latex(ots, runparams);
	docstring str = ss.str();
	if (optional && str.find(']') != docstring::npos)
		str = '{' + str + '}';
	os << (optional ? '[' : '{') << str
	   << (optional ? ']' : '}');
}


} // namespace lyx
