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

using std::endl;
using std::pair;
using lyx::pos_type;
using lyx::textclass_type;

extern BufferView * current_view;

// Jürgen, note that this means that you cannot currently have a list
// of selections cut/copied. So IMHO later we should have a
// list/vector/deque that we could store
// struct selection_item {
//       Paragraph * buf;
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


bool CutAndPaste::cutSelection(Paragraph * startpar, Paragraph * endpar,
			       int start, int & end, textclass_type tc,
			       bool doclear, bool realcut)
{
	if (!startpar || (start > startpar->size()))
		return false;

	if (realcut) {
		copySelection(startpar, endpar, start, end, tc);
	}

	if (!endpar || startpar == endpar) {
		if (startpar->erase(start, end)) {
			// Some chars were erased, go to start to be safe
			end = start;
		}
		return true;
	}

	bool actually_erased = false;

	// clear end/begin fragments of the first/last par in selection
	actually_erased |= (startpar)->erase(start, startpar->size());
	if (endpar->erase(0, end)) {
		actually_erased = true;
		end = 0;
	}

	// Loop through the deleted pars if any, erasing as needed

	Paragraph * pit = startpar->next();

	while (true) {
		// *endpar can be 0
		if (!pit)
			break;

		Paragraph * next = pit->next();

		// "erase" the contents of the par
		if (pit != endpar) {
			actually_erased |= pit->erase(0, pit->size());

			// remove the par if it's now empty
			if (actually_erased) {
				pit->previous()->next(pit->next());
				if (next) {
					next->previous(pit->previous());
				}

				delete pit;
			}
		}

		if (pit == endpar)
			break;

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

	if (!startpar->next())
		return true;

	Buffer * buffer = current_view->buffer();

	if (doclear) {
		startpar->next()->stripLeadingSpaces();
	}

	if (!actually_erased)
		return true;

	// paste the paragraphs again, if possible
	if (startpar->hasSameLayout(startpar->next()) ||
	    startpar->next()->empty()) {
#warning This is suspect. (Lgb)
		// When doing this merge we must know if the par really
		// belongs to an inset, and if it does then we have to use
		// the insets paragraphs, and not the buffers. (Lgb)
		mergeParagraph(buffer->params, buffer->paragraphs, startpar);
		// this because endpar gets deleted here!
		endpar = startpar;
	}

	return true;
}


bool CutAndPaste::copySelection(Paragraph * startpar, Paragraph * endpar,
				int start, int end, textclass_type tc)
{
	if (!startpar || (start > startpar->size()))
		return false;

	paragraphs.clear();

	textclass = tc;

	if (!endpar || startpar == endpar) {
		// only within one paragraph
		ParagraphList::iterator buf =
			paragraphs.insert(paragraphs.begin(), new Paragraph);

		buf->layout(startpar->layout());
		pos_type i = start;
		if (end > startpar->size())
			end = startpar->size();
		for (; i < end; ++i) {
			startpar->copyIntoMinibuffer(*current_view->buffer(), i);
			buf->insertFromMinibuffer(buf->size());
		}
	} else {
		// copy more than one paragraph
		// clone the paragraphs within the selection
		Paragraph * tmppar = startpar;

		while (tmppar != endpar) {
			Paragraph * newpar = new Paragraph(*tmppar, false);
			// reset change info
			newpar->cleanChanges();
			newpar->setInsetOwner(0);

			paragraphs.push_back(newpar);
			tmppar = tmppar->next();
		}

		// The first paragraph is too big.
		Paragraph & front = paragraphs.front();
		pos_type tmpi2 = start;
		for (; tmpi2; --tmpi2)
			front.erase(0);

		// Now last paragraph is too big, delete all after end.
		Paragraph & back = paragraphs.back();
		tmpi2 = end;
		while (back.size() > tmpi2) {
			back.erase(back.size() - 1);
		}
	}
	return true;
}


bool CutAndPaste::pasteSelection(Paragraph ** par, Paragraph ** endpar,
				 int & pos, textclass_type tc)
{
	if (!checkPastePossible())
		return false;

	if (pos > (*par)->size())
		pos = (*par)->size();

	// many paragraphs

	// make a copy of the simple cut_buffer
#if 1
	ParagraphList::iterator it = paragraphs.begin();

	ParagraphList simple_cut_clone;
	simple_cut_clone.insert(simple_cut_clone.begin(),
				new Paragraph(*it, false));

	ParagraphList::iterator end = paragraphs.end();
	while (boost::next(it) != end) {
		++it;
		simple_cut_clone.insert(simple_cut_clone.end(),
					new Paragraph(*it, false));
	}
#else
	// Later we want it done like this:
	ParagraphList simple_cut_clone(paragraphs.begin(),
				       paragraphs.end());
#endif
	// now remove all out of the buffer which is NOT allowed in the
	// new environment and set also another font if that is required
	ParagraphList::iterator tmpbuf = paragraphs.begin();
	int depth_delta = (*par)->params().depth() - tmpbuf->params().depth();
	// Temporary set *par as previous of tmpbuf as we might have
	// to realize the font.
	tmpbuf->previous(*par);

	// make sure there is no class difference
	SwitchLayoutsBetweenClasses(textclass, tc, &*tmpbuf,
				    current_view->buffer()->params);

	Paragraph::depth_type max_depth = (*par)->getMaxDepthAfter();

	while (tmpbuf != paragraphs.end()) {
		// If we have a negative jump so that the depth would
		// go below 0 depth then we have to redo the delta to
		// this new max depth level so that subsequent
		// paragraphs are aligned correctly to this paragraph
		// at level 0.
		if ((int(tmpbuf->params().depth()) + depth_delta) < 0)
			depth_delta = 0;
		// set the right depth so that we are not too deep or shallow.
		tmpbuf->params().depth(tmpbuf->params().depth() + depth_delta);
		if (tmpbuf->params().depth() > max_depth)
			tmpbuf->params().depth(max_depth);
		// only set this from the 2nd on as the 2nd depends for maxDepth
		// still on *par
		if (tmpbuf->previous() != (*par))
			max_depth = tmpbuf->getMaxDepthAfter();
		// set the inset owner of this paragraph
		tmpbuf->setInsetOwner((*par)->inInset());
		for (pos_type i = 0; i < tmpbuf->size(); ++i) {
			if (tmpbuf->getChar(i) == Paragraph::META_INSET) {
				if (!(*par)->insetAllowed(tmpbuf->getInset(i)->lyxCode())) {
					tmpbuf->erase(i--);
				}
			} else {
				LyXFont f1 = tmpbuf->getFont(current_view->buffer()->params, i, outerFont(tmpbuf));
				LyXFont f2 = f1;
				if (!(*par)->checkInsertChar(f1)) {
					tmpbuf->erase(i--);
				} else if (f1 != f2) {
					tmpbuf->setFont(i, f1);
				}
			}
		}
		tmpbuf = tmpbuf->next();
	}

	// now reset it to 0
	paragraphs.begin()->previous(0);

	// make the buf exactly the same layout than
	// the cursor paragraph
	paragraphs.begin()->makeSameLayout(*par);

	// find the end of the buffer
	ParagraphList::iterator lastbuffer = paragraphs.begin();
	while (boost::next(lastbuffer) != paragraphs.end())
		++lastbuffer;

	bool paste_the_end = false;

	// open the paragraph for inserting the buf
	// if necessary
	if (((*par)->size() > pos) || !(*par)->next()) {
		breakParagraphConservative(
			current_view->buffer()->params, current_view->buffer()->paragraphs, *par, pos);
		paste_the_end = true;
	}
	// set the end for redoing later
	*endpar = (*par)->next()->next();

	// paste it!
	lastbuffer->next((*par)->next());
	(*par)->next()->previous(&*lastbuffer);

	(*par)->next(&*paragraphs.begin());
	paragraphs.begin()->previous(*par);

	if ((*par)->next() == lastbuffer)
		lastbuffer = *par;

	mergeParagraph(current_view->buffer()->params,
		       current_view->buffer()->paragraphs, *par);
	// store the new cursor position
	*par = &*lastbuffer;
	pos = lastbuffer->size();
	// maybe some pasting
	if (lastbuffer->next() && paste_the_end) {
		if (lastbuffer->next()->hasSameLayout(&*lastbuffer)) {
			mergeParagraph(current_view->buffer()->params,
				       current_view->buffer()->paragraphs, lastbuffer);
		} else if (!lastbuffer->next()->size()) {
			lastbuffer->next()->makeSameLayout(&*lastbuffer);
			mergeParagraph(current_view->buffer()->params, current_view->buffer()->paragraphs, lastbuffer);
		} else if (!lastbuffer->size()) {
			lastbuffer->makeSameLayout(lastbuffer->next());
			mergeParagraph(current_view->buffer()->params,
				       current_view->buffer()->paragraphs, lastbuffer);
		} else
			lastbuffer->next()->stripLeadingSpaces();
	}
	// restore the simple cut buffer
	paragraphs = simple_cut_clone;

	return true;
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
	int ret = 0;
	if (!par || c1 == c2)
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
