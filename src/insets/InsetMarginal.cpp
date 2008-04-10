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

#include <ostream>


namespace lyx {


InsetMarginal::InsetMarginal(Buffer const & buf)
	: InsetFootlike(buf)
{}


docstring InsetMarginal::editMessage() const
{
	return _("Opened Marginal Note Inset");
}


docstring InsetMarginal::toolTip(BufferView const & bv, int x, int y) const
{
	docstring default_tip = InsetCollapsable::toolTip(bv, x, y);
	OutputParams rp(&buffer().params().encoding());
	odocstringstream ods;
	InsetText::plaintext(ods, rp);
	docstring margin_tip = ods.str();
	// shorten it if necessary
	if (margin_tip.size() > 200)
		margin_tip = margin_tip.substr(0, 200) + "...";
	if (!isOpen() && !margin_tip.empty())
		return margin_tip + '\n' + default_tip;
	return default_tip;
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


void InsetMarginal::addToToc(ParConstIterator const & cpit) const
{
	ParConstIterator pit = cpit;
	pit.push_back(*this);

	Toc & toc = buffer().tocBackend().toc("marginalnote");
	docstring str;
	str = getNewLabel(str);
	toc.push_back(TocItem(pit, 0, str));
}

} // namespace lyx
