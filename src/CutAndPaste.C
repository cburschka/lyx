/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "CutAndPaste.h"
//#include "debug.h"
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

#include "insets/inseterror.h"

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

Paragraph * buf = 0;
textclass_type textclass = 0;

// for now here this should be in another Cut&Paste Class!
// Jürgen, I moved this out of CutAndPaste since it does not operate on any
// member of the CutAndPaste class and in addition it was private.
// Perhaps it even should take a parameter? (Lgb)
void DeleteBuffer()
{
	if (!buf)
		return;

	Paragraph * tmppar;

	while (buf) {
		tmppar =  buf;
		buf = buf->next();
		delete tmppar;
	}
	buf = 0;
}

} // namespace anon


bool CutAndPaste::cutSelection(Paragraph * startpar, Paragraph ** endpar,
			       int start, int & end, char tc, bool doclear,
							   bool realcut)
{
	if (!startpar || (start > startpar->size()))
		return false;

	if (realcut)
		DeleteBuffer();

	textclass = tc;

	if (!(*endpar) || startpar == (*endpar)) {
		// only within one paragraph
		if (realcut) {
			buf = new Paragraph;
			buf->layout(startpar->layout());
		}
		pos_type i = start;
		if (end > startpar->size())
			end = startpar->size();
		for (; i < end; ++i) {
			if (realcut)
				startpar->copyIntoMinibuffer(*current_view->buffer(),
							     start);
			startpar->erase(start);
			if (realcut)
				buf->insertFromMinibuffer(buf->size());
		}
		end = start - 1;
	} else {
		// more than one paragraph
		breakParagraphConservative(current_view->buffer()->params,
					   *endpar,
					   end);
		*endpar = (*endpar)->next();
		end = 0;

		breakParagraphConservative(current_view->buffer()->params,
					   startpar,
					   start);

		// store the selection
		if (realcut) {
			buf = startpar->next();
			buf->previous(0);
		} else {
			startpar->next()->previous(0);
		}
		(*endpar)->previous()->next(0);

		// cut the selection
		startpar->next(*endpar);

		(*endpar)->previous(startpar);

		// the cut selection should begin with standard layout
		if (realcut) {
			buf->params().clear();
			buf->bibkey = 0;
			buf->layout(current_view->buffer()->params.getLyXTextClass().defaultLayout());
		}

		// paste the paragraphs again, if possible
		if (doclear)
			startpar->next()->stripLeadingSpaces();
		if (startpar->hasSameLayout(startpar->next()) ||
		    startpar->next()->empty()) {
			mergeParagraph(current_view->buffer()->params, startpar);
			(*endpar) = startpar; // this because endpar gets deleted here!
		}
		// this paragraph's are of noone's owner!
		Paragraph * p = buf;
		while (p) {
			p->setInsetOwner(0);
			p = p->next();
		}
	}
	return true;
}


bool CutAndPaste::copySelection(Paragraph * startpar, Paragraph * endpar,
				int start, int end, char tc)
{
	if (!startpar || (start > startpar->size()))
		return false;

	DeleteBuffer();

	textclass = tc;

	if (!endpar || startpar == endpar) {
		// only within one paragraph
		buf = new Paragraph;
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
		buf = new Paragraph(*tmppar, false);
		Paragraph * tmppar2 = buf;

		while (tmppar != endpar
		       && tmppar->next()) {
			tmppar = tmppar->next();
			tmppar2->next(new Paragraph(*tmppar, false));
			tmppar2->next()->previous(tmppar2);
			tmppar2 = tmppar2->next();
		}
		tmppar2->next(0);

		// the buf paragraph is too big
		pos_type tmpi2 = start;
		for (; tmpi2; --tmpi2)
			buf->erase(0);

		// now tmppar 2 is too big, delete all after end
		tmpi2 = end;
		while (tmppar2->size() > tmpi2) {
			tmppar2->erase(tmppar2->size() - 1);
		}
		// this paragraph's are of noone's owner!
		tmppar = buf;
		while (tmppar) {
			tmppar->setInsetOwner(0);
			tmppar = tmppar->next();
		}
	}
	return true;
}


bool CutAndPaste::pasteSelection(Paragraph ** par, Paragraph ** endpar,
				 int & pos, char tc)
{
	if (!checkPastePossible(*par))
		return false;

	if (pos > (*par)->size())
		pos = (*par)->size();

#if 0
	// Paragraph * tmpbuf;
	Paragraph * tmppar = *par;
	int tmppos = pos;

	// There are two cases: cutbuffer only one paragraph or many
	if (!buf->next()) {
		// only within a paragraph
		Paragraph * tmpbuf = new Paragraph(*buf, false);

		// Some provisions should be done here for checking
		// if we are inserting at the beginning of a
		// paragraph. If there are a space at the beginning
		// of the text to insert and we are inserting at
		// the beginning of the paragraph the space should
		// be removed.
		while (buf->size()) {
			// This is an attempt to fix the
			// "never insert a space at the
			// beginning of a paragraph" problem.
			if (!tmppos && buf->isLineSeparator(0)) {
				buf->erase(0);
			} else {
				buf->cutIntoMinibuffer(current_view->buffer()->params, 0);
				buf->erase(0);
				if (tmppar->insertFromMinibuffer(tmppos))
					++tmppos;
			}
		}
		delete buf;
		buf = tmpbuf;
		*endpar = tmppar->next();
		pos = tmppos;
	} else
#endif
	{
		// many paragraphs

		// make a copy of the simple cut_buffer
		Paragraph * tmpbuf = buf;
		Paragraph * simple_cut_clone = new Paragraph(*tmpbuf, false);
		Paragraph * tmpbuf2 = simple_cut_clone;

		while (tmpbuf->next()) {
			tmpbuf = tmpbuf->next();
			tmpbuf2->next(new Paragraph(*tmpbuf, false));
			tmpbuf2->next()->previous(tmpbuf2);
			tmpbuf2 = tmpbuf2->next();
		}

		// now remove all out of the buffer which is NOT allowed in the
		// new environment and set also another font if that is required
		tmpbuf = buf;
		int depth_delta = (*par)->params().depth() - tmpbuf->params().depth();
		// temporary set *par as previous of tmpbuf as we might have to realize
		// the font.
		tmpbuf->previous(*par);

		// make sure there is no class difference
		SwitchLayoutsBetweenClasses(textclass, tc, tmpbuf,
					    current_view->buffer()->params);

		Paragraph::depth_type max_depth = (*par)->getMaxDepthAfter();

		while(tmpbuf) {
			// if we have a negative jump so that the depth would go below
			// 0 depth then we have to redo the delta to this new max depth
			// level so that subsequent paragraphs are aligned correctly to
			// this paragraph at level 0.
			if ((static_cast<int>(tmpbuf->params().depth()) + depth_delta) < 0)
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
			for(pos_type i = 0; i < tmpbuf->size(); ++i) {
				if (tmpbuf->getChar(i) == Paragraph::META_INSET) {
					if (!(*par)->insetAllowed(tmpbuf->getInset(i)->lyxCode()))
					{
						tmpbuf->erase(i--);
					}
				} else {
					LyXFont f1 = tmpbuf->getFont(current_view->buffer()->params,i);
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
		buf->previous(0);

		// make the buf exactly the same layout than
		// the cursor paragraph
		buf->makeSameLayout(*par);

		// find the end of the buffer
		Paragraph * lastbuffer = buf;
		while (lastbuffer->next())
			lastbuffer = lastbuffer->next();

		bool paste_the_end = false;

		// open the paragraph for inserting the buf
		// if necessary
		if (((*par)->size() > pos) || !(*par)->next()) {
			breakParagraphConservative(
				current_view->buffer()->params,
				*par,
				pos);
			paste_the_end = true;
		}
		// set the end for redoing later
		*endpar = (*par)->next()->next();

		// paste it!
		lastbuffer->next((*par)->next());
		(*par)->next()->previous(lastbuffer);

		(*par)->next(buf);
		buf->previous(*par);

		if ((*par)->next() == lastbuffer)
			lastbuffer = *par;

		mergeParagraph(current_view->buffer()->params, *par);
		// store the new cursor position
		*par = lastbuffer;
		pos = lastbuffer->size();
		// maybe some pasting
		if (lastbuffer->next() && paste_the_end) {
			if (lastbuffer->next()->hasSameLayout(lastbuffer)) {
				mergeParagraph(current_view->buffer()->params, lastbuffer);
			} else if (lastbuffer->next()->empty()) {
				lastbuffer->next()->makeSameLayout(lastbuffer);
				mergeParagraph(current_view->buffer()->params, lastbuffer);
			} else if (lastbuffer->empty()) {
				lastbuffer->makeSameLayout(lastbuffer->next());
				mergeParagraph(current_view->buffer()->params, lastbuffer);
			} else
				lastbuffer->next()->stripLeadingSpaces();
		}
		// restore the simple cut buffer
		buf = simple_cut_clone;
	}

	return true;
}


int CutAndPaste::nrOfParagraphs()
{
	if (!buf)
		return 0;

	int n = 1;
	Paragraph * tmppar = buf;
	while (tmppar->next()) {
		++n;
		tmppar = tmppar->next();
	}
	return n;
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

		if (!hasLayout)
			par->layout(tclass2.defaultLayout());

		if (!hasLayout && name != tclass1.defaultLayoutName()) {
			++ret;
			string const s = _("Layout had to be changed from\n")
				+ name + _(" to ")
				+ par->layout()->name()
				+ _("\nbecause of class conversion from\n")
				+ tclass1.name() + _(" to ")
				+ tclass2.name();
			freezeUndo();
			InsetError * new_inset = new InsetError(s);
			LyXText * txt = current_view->getLyXText();
			LyXCursor cur = txt->cursor;
			txt->setCursorIntern(current_view, par, 0);
			txt->insertInset(current_view, new_inset);
			txt->fullRebreak(current_view);
			txt->setCursorIntern(current_view, cur.par(), cur.pos());
			unFreezeUndo();
		}
	}
	return ret;
}


bool CutAndPaste::checkPastePossible(Paragraph *)
{
	if (!buf) return false;

	return true;
}
