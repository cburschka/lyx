/**
 *  \file GraphicsSupport.C
 *  Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsSupport.h"

#include "BufferView.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "frontends/Painter.h"


typedef std::list<VisibleParagraph> VPList;


VPList const getVisibleParagraphs(BufferView const & bv)
{
	// top_y is not const because it's reset by getRowNearY.
	int top_y = bv.text->first_y;
	Row const * row = bv.text->getRowNearY(top_y);

	int const bv_height = bv.painter().paperHeight();

	VPList vps;
	Row const * last_row = 0;

	for (int height = 0; row && height < bv_height; row = row->next()) {
		height += row->height();

		if (vps.empty() || vps.back().par != row->par()) {
			vps.push_back(VisibleParagraph(row->par(),
						       row->pos(),
						       row->par()->size()));
		}

		last_row = row;
	}

	// If the end of the final paragraph is not visble,
	// update vps.back().end
	if (last_row && last_row->next() &&
	    last_row->par() == last_row->next()->par()) {
		vps.back().end = last_row->next()->pos();
	}

	return vps;
}


namespace {

struct InsetVisibleInParagraph {
	InsetVisibleInParagraph(Inset const & inset) : inset_(inset) {}
	bool operator()(VisibleParagraph const & vp)
	{
		Paragraph * par = vp.par;
		InsetList::iterator it  = par->insetlist.begin();
		InsetList::iterator end = par->insetlist.end();

		// Can't refactor this as a functor because we rely on the
		// inset_iterator member function getPos().
		for (; it != end; ++it) {
			lyx::pos_type const pos = it.getPos();
			if (pos >= vp.start && pos <= vp.end) {
				if (it.getInset() == &inset_ ||
				    it.getInset()->getInsetFromID(inset_.id()) != 0)
					return true;
			}
		}
		return false;
	}

private:
	Inset const & inset_;
};

} // namespace anon


bool isInsetVisible(Inset const & inset, VPList const & vps)
{
	if (vps.empty())
		return false;

	VPList::const_iterator it  = vps.begin();
	VPList::const_iterator end = vps.end();

	it = std::find_if(it, end, InsetVisibleInParagraph(inset));

	return it != end;
}
