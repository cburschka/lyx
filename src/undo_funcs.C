/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "undo_funcs.h"
#include "lyxtext.h"
#include "funcrequest.h"
#include "BufferView.h"
#include "buffer.h"
#include "insets/updatableinset.h"
#include "insets/insettext.h"
#include "debug.h"
#include "support/LAssert.h"
#include "iterators.h"


/// The flag used by FinishUndo().
bool undo_finished;
/// Whether actions are not added to the undo stacks.
bool undo_frozen;

namespace {


/**
 * Finish the undo operation in the case there was no entry
 * on the stack to perform.
 */
void finishNoUndo(BufferView * bv)
{
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	finishUndo();
	bv->text->postPaint(0);
	unFreezeUndo();
}


// Returns false if no undo possible.
bool textHandleUndo(BufferView * bv, Undo & undo)
{
	Buffer * buf = bv->buffer();

	ParIterator const before = buf->getParFromID(undo.number_of_before_par);
	ParIterator const behind = buf->getParFromID(undo.number_of_behind_par);
	ParIterator const null   = buf->par_iterator_end();

	int const before_id = (before == null) ? -1 : before->id();
	int const behind_id = (behind == null) ? -1 : behind->id();
	int const inset_id  = undo.number_of_inset_id;

	Inset * inset = bv->buffer()->getInsetFromID(inset_id);
	LyXText * text = inset ? inset->getLyXText(bv) : bv->text;

	ParagraphList * plist = &bv->text->ownerParagraphs();
	if (inset) {
		ParagraphList * tmp = inset->getParagraphs(0);
		if (tmp && !tmp->empty())
			plist = tmp;
	}

	ParagraphList::iterator first;
	if (before == null) {
		// if there's no before take the beginning of parlist.
		first = plist->begin();
		text->setCursorIntern(plist->begin(), 0);
	} else {
		first = *before;
		++first;
	}
	int const first_id  = first->id();

	lyxerr << "\nhandle: before_id: " << before_id << "\n";
	lyxerr << "handle: first_id:  " << first_id  << "\n";
	lyxerr << "handle: behind_id: " << behind_id << "\n";
	lyxerr << "handle: inset_id: " << inset_id << "\n";

	// Set the right(new) inset-owner of the paragraph if there is any.
	UpdatableInset * in = 0;
	if (before != null)
		in = before->inInset();
	else if (inset_id >= 0) {
		Inset * inset = bv->buffer()->getInsetFromID(inset_id);
		in = static_cast<UpdatableInset *>(inset);
	}
	ParagraphList::iterator pit = undo.pars.begin();
	ParagraphList::iterator end = undo.pars.end();
	for ( ; pit != end; ++pit)
		pit->setInsetOwner(in);
	lyxerr << "in: " << in << "\n";
	lyxerr << "undo.pars.size(): " << undo.pars.size() << "\n";

	// remove stuff between first and behind
	if (behind == null) 
		plist->erase(first, plist->end());
	else
		plist->erase(first, *behind);
	lyxerr << "after erase\n";

	// re-create first
	if (before == null) {
		// if there's no before take the beginning of parlist.
		lyxerr << "no 'before'\n";
		first = plist->begin();
	} else {
		lyxerr << "have 'before'\n";
		first = *before;
		++first;
	}


	// inset saved paragraphs
	lyxerr << "undo.pars.size(): " << undo.pars.size() << "\n";
	plist->insert(first, undo.pars.begin(), undo.pars.end());
	lyxerr << "after insert\n";
	/*
		// A memory optimization for edit:
		// Only layout information
		// is stored in the undo. So restore
		// the text informations.
		if (undo.kind == Undo::EDIT) {
			undo.pars[par]->setContentsFromPar(*deletelist.back());
			++par;
		}
	*/

	// Set the cursor for redoing
	// if we have a par before the first.
	if (before != null) {
		Inset * it = before->inInset();
		LyXText * text = it ? it->getLyXText(bv) : bv->text;
		text->setCursorIntern(*before, 0);
	}

	UpdatableInset * it = 0;
	if (first != plist->end())
		it = first->inInset();
	lyxerr << "it: " << it << "\n";


	text->redoParagraphs(text->cursor, plist->end());

	ParIterator tmppar = bv->buffer()->getParFromID(inset_id);

	if (tmppar != null) {
		lyxerr << "tmppar: " << tmppar->id() << "\n";
		LyXText * t;
		Inset * it = tmppar->inInset();
		if (it) {
			FuncRequest cmd(bv, LFUN_INSET_EDIT, "left");
			it->localDispatch(cmd);
			t = it->getLyXText(bv);
		} else {
			t = bv->text;
		}
		t->setCursorIntern(*tmppar, undo.cursor_pos);
		// Clear any selection and set the selection
		// cursor for an evt. new selection.
		t->clearSelection();
		t->selection.cursor = t->cursor;
		t->updateCounters();
	} else {
		lyxerr << "tmppar == null \n";
	}

	if (it) {
		lyxerr << "fit cursor...\n";
		bv->fitCursor();
		bv->updateInset(it);
		bv->text->setCursorIntern(bv->text->cursor.par(),
					  bv->text->cursor.pos());
	}

	finishUndo();
	bv->text->postPaint(0);

	lyxerr << "finished  textHandleUndo...\n";
	return true;
}


void createUndo(BufferView * bv, Undo::undo_kind kind,
	int first_id, int last_id,
	limited_stack<Undo> & stack)
{
	Buffer * buf = bv->buffer();

	ParIterator null    = buf->par_iterator_end();
	ParIterator prev    = null;
	ParIterator before  = null;
	ParIterator first   = null;
	ParIterator last    = null;
	ParIterator behind  = null;

	for (ParIterator it = buf->par_iterator_begin(); it != null; ++it) {
		if (it->id() == first_id) {
			first = it;
			before = prev;
		}
		if (it->id() == last_id) {
			last = it;
			behind = last;
			++behind;
		}
		prev = it;
	}

	if (last == null)
		last = first;

	int const before_id = (before == null) ? -1 : before->id();
	int const behind_id = (behind == null) ? -1 : behind->id();
	int inset_id        = (first->inInset()) ? first->inInset()->id() : -1;

	lyxerr << "\ncreate: before_id: " << before_id << "\n";
	lyxerr << "create: first_id:  " << first_id  << "\n";
	lyxerr << "create: last_id:   " << last_id   << "\n";
	lyxerr << "create: behind_id: " << behind_id << "\n";
	lyxerr << "create: inset_id:  " << inset_id  << "\n";
	lyxerr << "create: kind:  " << kind  << "\n";

	ParagraphList * plist = 0;
	if (first != null)
		plist = &first.plist();
	else if (behind != null)
		plist = &behind.plist();
	else if (!plist) {
		lyxerr << "plist from buffer (should this happen?)\n";
		plist = &buf->paragraphs;
	}

	// Undo::EDIT and Undo::FINISH are
	// always finished. (no overlapping there)
	// overlapping only with insert and delete inside one paragraph:
	// Nobody wants all removed  character
	// appear one by one when undoing.
	// EDIT is special since only layout information, not the
	// contents of a paragaph are stored.
	if (!undo_finished && kind != Undo::EDIT && kind != Undo::FINISH) {
		// Check whether storing is needed.
		if (!buf->undostack.empty() &&
		    buf->undostack.top().kind == kind &&
		    buf->undostack.top().number_of_before_par == before_id &&
		    buf->undostack.top().number_of_behind_par == behind_id) {
			// No undo needed.
			return;
		}
	}

	// Create a new Undo.
	LyXCursor const & cur = bv->theLockingInset() ?
			bv->theLockingInset()->cursor(bv) : bv->text->cursor;

	stack.push(Undo(kind, inset_id,
		before_id, behind_id, cur.par()->id(), cur.pos(), ParagraphList()));

	ParagraphList & undo_pars = stack.top().pars;

	for (ParagraphList::iterator it = *first; it != *last; ++it) {
		undo_pars.push_back(*it);
		undo_pars.back().id(it->id());
	}
	undo_pars.push_back(**last);
	undo_pars.back().id(last->id());

	// A memory optimization: Just store the layout
	// information when only edit.
#warning Waste...
	//if (kind == Undo::EDIT)
	//	for (size_t i = 0, n = undo_pars.size(); i < n; ++i)
	//		undo_pars[i]->clearContents();

	undo_finished = false;
}


// Returns false if no undo possible.
bool textUndoOrRedo(BufferView * bv,
	limited_stack<Undo> & stack,
	limited_stack<Undo> & /*otherstack*/)
{
	if (stack.empty()) {
		finishNoUndo(bv);
		return false;
	}

	Undo undo = stack.top();
	stack.pop();
	finishUndo();

/*
	if (!undo_frozen) {
		Buffer * buf = bv->buffer();
		ParIterator p = buf->getParFromID(undo->number_of_before_par);
		ParIterator const end = buf->par_iterator_end();
		bool ok = false;
		ParagraphList::iterator first;
		// default constructed?
		if (p != end) {
			first = p.par();
			if (first->next())
				first = first->next();
		} else {
			// Set first to the very first Paragraph depending of where
			// we are so it will return the first paragraph of the buffer or the
			// first paragraph of the textinset we're in.
			first = bv->text->ownerParagraphs()->begin();
			Inset * inset = bv->buffer()->getInsetFromID(inset_id);
			if (inset) {
				ParagraphList * result = inset->getParagraphs(0);
				if (result && !result->empty())
					first = result->begin();
			}
		}
		if (ok) {
			ParIterator behind = buf->getParFromID(undo.number_of_behind_par);
			createUndo(bv, undo.kind, first, behind.par(), otherstack);
		}
	}
*/

	// Now we can unlock the inset for saftey because the inset
	// pointer could be changed during the undo-function. Anyway
	// if needed we have to lock the right inset/position if this
	// is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool const ret = textHandleUndo(bv, undo);
	unFreezeUndo();
	return ret;
}

} // namespace anon


void finishUndo()
{
	// Makes sure the next operation will be stored.
	undo_finished = true;
}


void freezeUndo()
{
	// This is dangerous and for internal use only.
	undo_frozen = true;
}


void unFreezeUndo()
{
	// This is dangerous and for internal use only.
	undo_frozen = false;
}


bool textUndo(BufferView * bv)
{
	return textUndoOrRedo(bv, bv->buffer()->undostack,
			      bv->buffer()->redostack);
}


bool textRedo(BufferView * bv)
{
	return textUndoOrRedo(bv, bv->buffer()->redostack,
			      bv->buffer()->undostack);
}


void setUndo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first)
{
	setUndo(bv, kind, first, first);
}


void setUndo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first, ParagraphList::iterator last)
{
	if (!undo_frozen) {
		createUndo(bv, kind, first->id(), last->id(), bv->buffer()->undostack);
		bv->buffer()->redostack.clear();
	}
}


void setCursorParUndo(BufferView * bv)
{
	setUndo(bv, Undo::FINISH, bv->text->cursor.par());
}
