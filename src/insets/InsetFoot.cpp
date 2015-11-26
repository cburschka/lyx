/**
 * \file InsetFoot.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFoot.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Language.h"
#include "Layout.h"
#include "OutputParams.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {

InsetFoot::InsetFoot(Buffer * buf)
	: InsetFootlike(buf), intitle_(false)
{}


docstring InsetFoot::layoutName() const
{
	return intitle_ ? from_ascii("Foot:InTitle") : from_ascii("Foot");
}


void InsetFoot::updateBuffer(ParIterator const & it, UpdateType utype)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	Counters & cnts = bp.documentClass().counters();
	if (utype == OutputUpdate) {
		// the footnote counter is local to this inset
		cnts.saveLastCounter();
	}

	intitle_ = false;
	for (size_type sl = 0 ; sl < it.depth() ; ++ sl) {
		if (it[sl].text() && it[sl].paragraph().layout().intitle) {
			intitle_ = true;
			break;
		}
	}

	Language const * lang = it.paragraph().getParLanguage(bp);
	InsetLayout const & il = getLayout();
	docstring const & count = il.counter();
	custom_label_ = translateIfPossible(il.labelstring());
	if (cnts.hasCounter(count))
		cnts.step(count, utype);
	custom_label_ += ' ' + cnts.theCounter(count, lang->code());
	setLabel(custom_label_);

	InsetCollapsable::updateBuffer(it, utype);
	if (utype == OutputUpdate)
		cnts.restoreLastCounter();
}


void InsetFoot::addToToc(DocIterator const & cpit, bool output_active,
						 UpdateType utype) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetFoot &>(*this)));
	
	docstring tooltip;
	text().forOutliner(tooltip, TOC_ENTRY_LENGTH);
	docstring str = custom_label_ + ": " + tooltip;
	tooltip = support::wrapParas(tooltip, 0, 60, 2);
	
	shared_ptr<Toc> toc = buffer().tocBackend().toc("footnote");
	toc->push_back(TocItem(pit, 0, str, output_active, tooltip));
	
	// Proceed with the rest of the inset.
	InsetFootlike::addToToc(cpit, output_active, utype);
}


docstring InsetFoot::toolTip(BufferView const & bv, int x, int y) const
{
	if (isOpen(bv))
		// this will give us something useful if there is no button
		return InsetCollapsable::toolTip(bv, x, y);
	return toolTipText(custom_label_+ ": ");
}


int InsetFoot::plaintext(odocstringstream & os,
        OutputParams const & runparams, size_t max_length) const
{
	os << '[' << buffer().B_("footnote") << ":\n";
	InsetText::plaintext(os, runparams, max_length);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetFoot::docbook(odocstream & os, OutputParams const & runparams) const
{
	os << "<footnote>";
	int const i = InsetText::docbook(os, runparams);
	os << "</footnote>";

	return i;
}

} // namespace lyx
