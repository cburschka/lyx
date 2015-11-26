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
#include "support/lstrings.h"

namespace lyx {


InsetMarginal::InsetMarginal(Buffer * buf)
	: InsetFootlike(buf)
{}


int InsetMarginal::plaintext(odocstringstream & os,
			     OutputParams const & runparams, size_t max_length) const
{
	os << '[' << buffer().B_("margin") << ":\n";
	InsetText::plaintext(os, runparams, max_length);
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


void InsetMarginal::addToToc(DocIterator const & cpit, bool output_active,
				  UpdateType utype) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetMarginal &>(*this)));

	docstring tooltip;
	text().forOutliner(tooltip, TOC_ENTRY_LENGTH);
	docstring const str = tooltip;
	tooltip = support::wrapParas(tooltip, 0, 60, 2);
	
	shared_ptr<Toc> toc = buffer().tocBackend().toc("marginalnote");
	toc->push_back(TocItem(pit, 0, str, output_active, tooltip));

	// Proceed with the rest of the inset.
	InsetFootlike::addToToc(cpit, output_active, utype);
}

} // namespace lyx
