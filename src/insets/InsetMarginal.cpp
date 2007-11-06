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
#include "gettext.h"
#include "OutputParams.h"

#include <ostream>


namespace lyx {


InsetMarginal::InsetMarginal(BufferParams const & bp)
	: InsetFootlike(bp)
{}


InsetMarginal::InsetMarginal(InsetMarginal const & in)
	: InsetFootlike(in)
{}


Inset * InsetMarginal::clone() const
{
	return new InsetMarginal(*this);
}


docstring const InsetMarginal::editMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::latex(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams) const
{
	os << "%\n\\marginpar{";
	int const i = InsetText::latex(buf, os, runparams);
	os << "%\n}";
	return i + 2;
}


int InsetMarginal::plaintext(Buffer const & buf, odocstream & os,
			     OutputParams const & runparams) const
{
	os << '[' << buf.B_("margin") << ":\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetMarginal::docbook(Buffer const & buf, odocstream & os,
			   OutputParams const & runparams) const
{
	os << "<note role=\"margin\">";
	int const i = InsetText::docbook(buf, os, runparams);
	os << "</note>";

	return i;
}


} // namespace lyx
