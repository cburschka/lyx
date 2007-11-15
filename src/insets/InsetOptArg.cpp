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

#include "debug.h"
#include "gettext.h"

#include "support/docstream.h"


namespace lyx {


InsetOptArg::InsetOptArg(BufferParams const & ins)
	: InsetCollapsable(ins)
{}


InsetOptArg::InsetOptArg(InsetOptArg const & in)
	: InsetCollapsable(in)
{}


Inset * InsetOptArg::clone() const
{
	return new InsetOptArg(*this);
}


docstring const InsetOptArg::editMessage() const
{
	return _("Opened Optional Argument Inset");
}


void InsetOptArg::write(Buffer const & buf, std::ostream & os) const
{
	os << "OptArg" << "\n";
	InsetCollapsable::write(buf, os);
}


int InsetOptArg::latex(Buffer const &, odocstream &,
		       OutputParams const &) const
{
	return 0;
}


int InsetOptArg::plaintext(Buffer const &, odocstream &,
			   OutputParams const &) const
{
	return 0; // do not output optional arguments
}


int InsetOptArg::docbook(Buffer const &, odocstream &,
			 OutputParams const &) const
{
	return 0;
}


int InsetOptArg::latexOptional(Buffer const & buf, odocstream & os,
			       OutputParams const & runparams) const
{
	odocstringstream ss;
	int ret = InsetText::latex(buf, ss, runparams);
	docstring str = ss.str();
	if (str.find(']') != docstring::npos)
		str = '{' + str + '}';
	os << '[' << str << ']';
	return ret;
}


} // namespace lyx
