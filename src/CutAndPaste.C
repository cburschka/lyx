/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "CutAndPaste.h"
#include "BufferView.h"
#include "buffer.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "lyxtext.h"
#include "lyxcursor.h"
#include "gettext.h"
#include "iterators.h"
#include "lyxtextclasslist.h"
#include "undo_funcs.h"
#include "paragraph_funcs.h"
#include "debug.h"

#include "insets/inseterror.h"

#include "support/BoostFormat.h"
#include "support/LAssert.h"

using std::endl;
using std::pair;
using std::make_pair;
using std::for_each;

using lyx::pos_type;
using lyx::textclass_type;

extern BufferView * current_view;

// Jürgen, note that this means that you cannot currently have a list
// of selections cut/copied. So IMHO later we should have a
// list/vector/deque that we could store
// struct selection_item {
//       ParagraphList copy_pars;
//       LyXTextClassList::size_type textclass;
// };
// in and some method of choosing beween them (based on the first few chars
// in the selection probably.) This would be a nice feature and quite
// easy to implement. (Lgb)
//
// Sure but I just cleaned up this code for now with the same functionality
// as before. I also want to add a XClipboard function so that we can copy
// text from LyX to some other X-application in the form of ASCII or in the
// form of LaTeX (or Docbook depending on the document-class!). Think how nice
// it could be to select a math-inset do a "Copy to X-Clipboard as LaTeX" and
// then do a middle mouse button click in the application you want and have
// the whole formula there in LaTeX-Code. (Jug)

namespace {

// FIXME: stupid name
ParagraphList paragraphs;
textclass_type textclass = 0;

} // namespace anon

PitPosPair CutAndPaste::cutSelection(ParagraphList & pars,
				     ParagraphList::iterator startpit,
				     ParagraphList::iterator endpit,
				     int startpos, int endpos,
				     textclass_type tc, bool doclear)
{
	copySelection(startpit, endpit, startpos, endpos, tc);
	return eraseSelection(pars, startpit, endpit, startpos,
			      endpos, doclear);
}


PitPosPair CutAndPaste::eraseSelection(ParagraphList & pars,
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
#warning current_view used here.
// should we pass buffer or buffer->params around?
		Buffer * buffer = current_view->buffer();
		mergeParagraph(buffer->params, pars, &*startpit);
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
	lyx::Assert(&*startpit);
	lyx::Assert(&*endpit);
	lyx::Assert(0 <= start && start <= startpit->size());
	lyx::Assert(0 <= end && end <= endpit->size());
	lyx::Assert(startpit != endpit || start <= end);

	textclass = tc;

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

	return true;
}


pair<PitPosPair, ParagraphList::iterator>
CutAndPaste::pasteSelection(ParagraphList & pars,
			    ParagraphList::iterator pit, int pos,
			    textclass_type tc)
{
	if (!checkPastePossible())
		return make_pair(PitPosPair(pit, pos), pit);

	lyx::Assert (pos <= pit->size());

	// Make a copy of the CaP paragraphs.
	ParagraphList simple_cut_clone = paragraphs;

	// Now remove all out of the pars which is NOT allowed in the
	// new environment and set also another font if that is required.

	ParagraphList::iterator tmpbuf = simple_cut_clone.begin();
	int depth_delta = pit->params().depth() - tmpbuf->params().depth();

	// Make sure there is no class difference.
#warning current_view used here
	SwitchLayoutsBetweenClasses(textclass, tc, &*tmpbuf,
				    current_view->buffer()->params);

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
				LyXFont f1 = tmpbuf->getFont(current_view->buffer()->params, i, outerFont(pit, pars));
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

	bool paste_the_end = false;

	// Open the paragraph for inserting the buf
	// if necessary.
	if (pit->size() > pos || boost::next(pit) == pars.end()) {
		breakParagraphConservative(current_view->buffer()->params,
					   pars, pit, pos);
		paste_the_end = true;
	}

	// Set the end for redoing later.
	ParagraphList::iterator endpit = boost::next(boost::next(pit));

	// Paste it!

	ParagraphList::iterator past_pit = boost::next(pit);
	// It is possible that std::list::splice also could be used here.
	pars.insert(past_pit,
		    simple_cut_clone.begin(), simple_cut_clone.end());
	ParagraphList::iterator last_paste = boost::prior(past_pit);

	// If we only inserted one paragraph.
	if (boost::next(pit) == last_paste)
		last_paste = pit;

	mergeParagraph(current_view->buffer()->params, pars, pit);

	// Store the new cursor position.
	pit = last_paste;
	pos = last_paste->size();

	// Maybe some pasting.
#warning CHECK! Are we comparing last_paste to the wrong list here? (Lgb)
	if (boost::next(last_paste) != simple_cut_clone.end() &&
	    paste_the_end) {
		if (boost::next(last_paste)->hasSameLayout(*last_paste)) {
			mergeParagraph(current_view->buffer()->params, pars,
				       last_paste);
		} else if (boost::next(last_paste)->empty()) {
			boost::next(last_paste)->makeSameLayout(*last_paste);
			mergeParagraph(current_view->buffer()->params, pars,
				       last_paste);
		} else if (last_paste->empty()) {
			last_paste->makeSameLayout(*boost::next(last_paste));
			mergeParagraph(current_view->buffer()->params, pars,
				       last_paste);
		} else
			boost::next(last_paste)->stripLeadingSpaces();
	}

	return make_pair(PitPosPair(pit, pos), endpit);
}


int CutAndPaste::nrOfParagraphs()
{
	return paragraphs.size();
}


int CutAndPaste::SwitchLayoutsBetweenClasses(textclass_type c1,
					     textclass_type c2,
					     Paragraph * par,
					     BufferParams const & /*bparams*/)
{
	lyx::Assert(par);

	int ret = 0;
	if (c1 == c2)
		return ret;

	LyXTextClass const & tclass1 = textclasslist[c1];
	LyXTextClass const & tclass2 = textclasslist[c2];
	ParIterator end = ParIterator();
	for (ParIterator it = ParIterator(par); it != end; ++it) {
		par = *it;
		string const name = par->layout()->name();
		bool hasLayout = tclass2.hasLayout(name);

		if (hasLayout)
			par->layout(tclass2[name]);
		else
			par->layout(tclass2.defaultLayout());

		if (!hasLayout && name != tclass1.defaultLayoutName()) {
			++ret;
#if USE_BOOST_FORMAT
			boost::format fmt(_("Layout had to be changed from\n"
					    "%1$s to %2$s\n"
					    "because of class conversion from\n"
					    "%3$s to %4$s"));
			fmt     % name
				% par->layout()->name()
				% tclass1.name()
				% tclass2.name();

			string const s = fmt.str();
#else
			string const s = _("Layout had to be changed from\n")
				+ name + _(" to ")
				+ par->layout()->name()
				+ _("\nbecause of class conversion from\n")
				+ tclass1.name() + _(" to ")
				+ tclass2.name();
#endif
			freezeUndo();
			InsetError * new_inset = new InsetError(s);
			LyXText * txt = current_view->getLyXText();
			LyXCursor cur = txt->cursor;
			txt->setCursorIntern(par, 0);
			txt->insertInset(new_inset);
			txt->fullRebreak();
			txt->setCursorIntern(cur.par(), cur.pos());
			unFreezeUndo();
		}
	}
	return ret;
}


bool CutAndPaste::checkPastePossible()
{
	return !paragraphs.empty();
}
