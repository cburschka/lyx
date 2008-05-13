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
#include "BufferParams.h"
#include "OutputParams.h"
#include "TocBackend.h"

#include "support/docstream.h"
#include "support/gettext.h"

namespace lyx {


InsetMarginal::InsetMarginal(Buffer const & buf)
	: InsetFootlike(buf)
{}


docstring InsetMarginal::editMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::latex(odocstream & os, OutputParams const & runparams) const
{
	os << "%\n\\marginpar{";
	int const i = InsetText::latex(os, runparams);
	os << "%\n}";
	return i + 2;
}


int InsetMarginal::plaintext(odocstream & os,
			     OutputParams const & runparams) const
{
	os << '[' << buffer().B_("margin") << ":\n";
	InsetText::plaintext(os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetMarginal::docbook(odocstream & os,
			   OutputParams const & runparams) const
{
	os << "<note role=\"margin\">";
	int const i = InsetText::docbook(os, runparams);
	os << "</note>";

	return i;
}


void InsetMarginal::addToToc(DocIterator const & cpit)
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(*this));

	Toc & toc = buffer().tocBackend().toc("marginalnote");
	docstring str;
	str = getNewLabel(str);
	toc.push_back(TocItem(pit, 0, str));
}

} // namespace lyx
