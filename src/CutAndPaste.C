/*
 * \file CutAndPaste.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CutAndPaste.h"

#include "buffer.h"
#include "bufferparams.h"
#include "errorlist.h"
#include "gettext.h"
#include "iterators.h"
#include "lyxtextclasslist.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"

#include "insets/insettabular.h"

#include "support/lstrings.h"

using lyx::pos_type;
using lyx::textclass_type;

using lyx::support::bformat;

using std::for_each;
using std::make_pair;
using std::pair;
using std::vector;
using std::string;


typedef limited_stack<pair<ParagraphList, textclass_type> > CutStack;

namespace {

CutStack cuts(10);

} // namespace anon


std::vector<string>
CutAndPaste::availableSelections(Buffer const & buffer)
{
	vector<string> selList;

	CutStack::const_iterator cit = cuts.begin();
	CutStack::const_iterator end = cuts.end();
	for (; cit != end; ++cit) {
		// we do not use cit-> here because gcc 2.9x does not
		// like it (JMarc)
		ParagraphList const & pars = (*cit).first;
		string asciiSel;
		ParagraphList::const_iterator pit = pars.begin();
		ParagraphList::const_iterator pend = pars.end();
		for (; pit != pend; ++pit) {
			asciiSel += pit->asString(buffer, false);
			if (asciiSel.size() > 25) {
				asciiSel.replace(22, string::npos, "...");
				break;
			}
		}

		selList.push_back(asciiSel);
	}

	return selList;
}


PitPosPair CutAndPaste::cutSelection(BufferParams const & params,
				     ParagraphList & pars,
				     ParagraphList::iterator startpit,
				     ParagraphList::iterator endpit,
				     int startpos, int endpos,
				     textclass_type tc, bool doclear)
{
	copySelection(startpit, endpit, startpos, endpos, tc);
	return eraseSelection(params, pars, startpit, endpit, startpos,
			      endpos, doclear);
}


PitPosPair CutAndPaste::eraseSelection(BufferParams const & params,
				       ParagraphList & pars,
				       ParagraphList::iterator startpit,
				       ParagraphList::iterator endpit,
				       int startpos, int endpos, bool doclear)
{
	if (startpit == pars.end() || (startpos > startpit->size()))
		return PitPosPair(endpit, endpos);

	if (endpit == pars.end() || startpit == endpit) {
		endpos -= startpit->erase(startpos, endpos);
		return PitPosPair(endpit, endpos);
	}

	// clear end/begin fragments of the first/last par in selection
	bool all_erased = true;

	startpit->erase(startpos, startpit->size());
	if (startpit->size() != startpos)
		all_erased = false;

	endpos -= endpit->erase(0, endpos);
	if (endpos != 0)
		all_erased = false;

	// Loop through the deleted pars if any, erasing as needed

	ParagraphList::iterator pit = boost::next(startpit);

	while (pit != endpit && pit != pars.end()) {
		ParagraphList::iterator const next = boost::next(pit);
		// "erase" the contents of the par
		pit->erase(0, pit->size());
		if (!pit->size()) {
			// remove the par if it's now empty
			pars.erase(pit);
		} else
			all_erased = false;
		pit = next;
	}

#if 0 // FIXME: why for cut but not copy ?
	// the cut selection should begin with standard layout
	if (realcut) {
		buf->params().clear();
		buf->bibkey = 0;
		buf->layout(textclasslist[buffer->params.textclass].defaultLayoutName());
	}
#endif

	if (boost::next(startpit) == pars.end())
		return PitPosPair(endpit, endpos);

	if (doclear) {
		boost::next(startpit)->stripLeadingSpaces();
	}

	// paste the paragraphs again, if possible
	if (all_erased &&
	    (startpit->hasSameLayout(*boost::next(startpit)) ||
	     boost::next(startpit)->empty())) {
		mergeParagraph(params, pars, startpit);
		// this because endpar gets deleted here!
		endpit = startpit;
		endpos = startpos;
	}

	return PitPosPair(endpit, endpos);

}


namespace {

struct resetOwnerAndChanges {
	void operator()(Paragraph & p) {
		p.cleanChanges();
		p.setInsetOwner(0);
	}
};

} // anon namespace

bool CutAndPaste::copySelection(ParagraphList::iterator startpit,
				ParagraphList::iterator endpit,
				int start, int end, textclass_type tc)
{
	BOOST_ASSERT(0 <= start && start <= startpit->size());
	BOOST_ASSERT(0 <= end && end <= endpit->size());
	BOOST_ASSERT(startpit != endpit || start <= end);

	ParagraphList paragraphs;

	// Clone the paragraphs within the selection.
	ParagraphList::iterator postend = boost::next(endpit);

	paragraphs.assign(startpit, postend);
	for_each(paragraphs.begin(), paragraphs.end(), resetOwnerAndChanges());

	// Cut out the end of the last paragraph.
	Paragraph & back = paragraphs.back();
	back.erase(end, back.size());

	// Cut out the begin of the first paragraph
	Paragraph & front = paragraphs.front();
	front.erase(0, start);

	cuts.push(make_pair(paragraphs, tc));

	return true;
}


pair<PitPosPair, ParagraphList::iterator>
CutAndPaste::pasteSelection(Buffer const & buffer,
			    ParagraphList & pars,
			    ParagraphList::iterator pit, int pos,
			    textclass_type tc,
			    ErrorList & errorlist)
{
	return pasteSelection(buffer, pars, pit, pos, tc, 0, errorlist);
}


pair<PitPosPair, ParagraphList::iterator>
CutAndPaste::pasteSelection(Buffer const & buffer,
			    ParagraphList & pars,
			    ParagraphList::iterator pit, int pos,
			    textclass_type tc, size_t cut_index,
			    ErrorList & errorlist)
{
	if (!checkPastePossible())
		return make_pair(PitPosPair(pit, pos), pit);

	BOOST_ASSERT (pos <= pit->size());

	// Make a copy of the CaP paragraphs.
	ParagraphList simple_cut_clone = cuts[cut_index].first;
	textclass_type const textclass = cuts[cut_index].second;

	// Now remove all out of the pars which is NOT allowed in the
	// new environment and set also another font if that is required.

	// Make sure there is no class difference.
	SwitchLayoutsBetweenClasses(textclass, tc, simple_cut_clone,
				    errorlist);

	ParagraphList::iterator tmpbuf = simple_cut_clone.begin();
	int depth_delta = pit->params().depth() - tmpbuf->params().depth();

	Paragraph::depth_type max_depth = pit->getMaxDepthAfter();

	for (; tmpbuf != simple_cut_clone.end(); ++tmpbuf) {
		// If we have a negative jump so that the depth would
		// go below 0 depth then we have to redo the delta to
		// this new max depth level so that subsequent
		// paragraphs are aligned correctly to this paragraph
		// at level 0.
		if ((int(tmpbuf->params().depth()) + depth_delta) < 0)
			depth_delta = 0;

		// Set the right depth so that we are not too deep or shallow.
		tmpbuf->params().depth(tmpbuf->params().depth() + depth_delta);
		if (tmpbuf->params().depth() > max_depth)
			tmpbuf->params().depth(max_depth);

		// Only set this from the 2nd on as the 2nd depends
		// for maxDepth still on pit.
		if (tmpbuf != simple_cut_clone.begin())
			max_depth = tmpbuf->getMaxDepthAfter();

		// Set the inset owner of this paragraph.
		tmpbuf->setInsetOwner(pit->inInset());
		for (pos_type i = 0; i < tmpbuf->size(); ++i) {
			if (tmpbuf->getChar(i) == Paragraph::META_INSET) {
				if (!pit->insetAllowed(tmpbuf->getInset(i)->lyxCode())) {
					tmpbuf->erase(i--);
				}
			} else {
				LyXFont f1 = tmpbuf->getFont(buffer.params(), i, outerFont(pit, pars));
				LyXFont f2 = f1;
				if (!pit->checkInsertChar(f1)) {
					tmpbuf->erase(i--);
				} else if (f1 != f2) {
					tmpbuf->setFont(i, f1);
				}
			}
		}
	}

	// Make the buf exactly the same layout than
	// the cursor paragraph.
	simple_cut_clone.begin()->makeSameLayout(*pit);

	// Prepare the paragraphs and insets for insertion
	// A couple of insets store buffer references so need
	// updating
	ParIterator fpit(simple_cut_clone.begin(), simple_cut_clone);
	ParIterator fend(simple_cut_clone.end(), simple_cut_clone);

	for (; fpit != fend; ++fpit) {
		InsetList::iterator lit = fpit->insetlist.begin();
		InsetList::iterator eit = fpit->insetlist.end();

		for (; lit != eit; ++lit) {
			switch (lit->inset->lyxCode()) {
			case InsetOld::TABULAR_CODE: {
				InsetTabular * it = static_cast<InsetTabular*>(lit->inset);
				it->buffer(const_cast<Buffer*>(&buffer));
				break;
			}

			default:
				break; // nothing
			}
		}
	}

	bool paste_the_end = false;

	// Open the paragraph for inserting the buf
	// if necessary.
	if (pit->size() > pos || boost::next(pit) == pars.end()) {
		breakParagraphConservative(buffer.params(),
					   pars, pit, pos);
		paste_the_end = true;
	}

	// Set the end for redoing later.
	ParagraphList::iterator endpit = boost::next(boost::next(pit));

	// Paste it!

	ParagraphList::iterator past_pit = boost::next(pit);
	pars.splice(past_pit, simple_cut_clone);
	ParagraphList::iterator last_paste = boost::prior(past_pit);

	// If we only inserted one paragraph.
	if (boost::next(pit) == last_paste)
		last_paste = pit;

	mergeParagraph(buffer.params(), pars, pit);

	// Store the new cursor position.
	pit = last_paste;
	pos = last_paste->size();

	// Maybe some pasting.
	if (boost::next(last_paste) != pars.end() &&
	    paste_the_end) {
		if (boost::next(last_paste)->hasSameLayout(*last_paste)) {
			mergeParagraph(buffer.params(), pars,
				       last_paste);
		} else if (boost::next(last_paste)->empty()) {
			boost::next(last_paste)->makeSameLayout(*last_paste);
			mergeParagraph(buffer.params(), pars,
				       last_paste);
		} else if (last_paste->empty()) {
			last_paste->makeSameLayout(*boost::next(last_paste));
			mergeParagraph(buffer.params(), pars,
				       last_paste);
		} else
			boost::next(last_paste)->stripLeadingSpaces();
	}

	return make_pair(PitPosPair(pit, pos), endpit);
}


int CutAndPaste::nrOfParagraphs()
{
	return cuts.empty() ? 0 : cuts[0].first.size();
}


int CutAndPaste::SwitchLayoutsBetweenClasses(textclass_type c1,
					     textclass_type c2,
					     ParagraphList & pars,
					     ErrorList & errorlist)
{
	BOOST_ASSERT(!pars.empty());

	int ret = 0;
	if (c1 == c2)
		return ret;

	LyXTextClass const & tclass1 = textclasslist[c1];
	LyXTextClass const & tclass2 = textclasslist[c2];
	ParIterator end = ParIterator(pars.end(), pars);
	for (ParIterator it = ParIterator(pars.begin(), pars); it != end; ++it) {
		string const name = it->layout()->name();
		bool hasLayout = tclass2.hasLayout(name);

		if (hasLayout)
			it->layout(tclass2[name]);
		else
			it->layout(tclass2.defaultLayout());

		if (!hasLayout && name != tclass1.defaultLayoutName()) {
			++ret;
			string const s = bformat(
				_("Layout had to be changed from\n%1$s to %2$s\n"
				"because of class conversion from\n%3$s to %4$s"),
			 name, it->layout()->name(), tclass1.name(), tclass2.name());
			// To warn the user that something had to be done.
			errorlist.push_back(ErrorItem("Changed Layout", s,
						      it->id(), 0,
						      it->size()));
		}
	}
	return ret;
}


bool CutAndPaste::checkPastePossible()
{
	return !cuts.empty() && !cuts[0].first.empty();
}
