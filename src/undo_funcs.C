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

#include <vector>

using std::vector;
using boost::shared_ptr;


/// The flag used by FinishUndo().
bool undo_finished;
/// Whether actions are not added to the undo stacks.
bool undo_frozen;

namespace {

/// Utility to return the cursor.
LyXCursor const & undoCursor(BufferView * bv)
{
	if (bv->theLockingInset())
		return bv->theLockingInset()->cursor(bv);
	return bv->text->cursor;
}


/**
 * Returns a pointer to the very first Paragraph depending of where
 * we are so it will return the first paragraph of the buffer or the
 * first paragraph of the textinset we're in.
 */
ParagraphList * undoParagraphs(BufferView * bv, int inset_id)
{
	Inset * inset = bv->buffer()->getInsetFromID(inset_id);
	if (inset) {
		ParagraphList * result = inset->getParagraphs(0);
		if (result && !result->empty())
			return result;
	}
	return &bv->text->ownerParagraphs();
}


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
	Buffer * b = bv->buffer();

	ParIterator const before = b->getParFromID(undo.number_of_before_par);
	ParIterator const behind = b->getParFromID(undo.number_of_behind_par);
	ParIterator const null   = b->par_iterator_end();

	int const before_id = (before == null) ? -1 : before->id();
	int const behind_id = (behind == null) ? -1 : behind->id();
	int const inset_id  = undo.number_of_inset_id;

	ParagraphList * plist = undoParagraphs(bv, inset_id);

	ParagraphList::iterator first;
	if (before == null) {
		// if there's no before take the beginning of parlist.
		first = plist->begin();
		LyXText * t = bv->text;
		if (inset_id >= 0)
			if (Inset * in = bv->buffer()->getInsetFromID(inset_id))
				t = in->getLyXText(bv);
		t->setCursorIntern(plist->begin(), 0);
	} else {
		first = *before;
		++first;
	}
	int const first_id  = first->id();

	int after_id;
	ParagraphList::iterator after;
	if (behind == null) {
		// if there's no behind take the end of parlist.
		after = plist->end();
		after_id = -1;
	} else {
		after = *behind;
		after_id = after->id();
	}

	lyxerr << "\nbefore_id: " << before_id << "\n";
	lyxerr << "first_id:  " << first_id  << "\n";
	lyxerr << "after_id: " << after_id << "\n";
	lyxerr << "behind_id: " << behind_id << "\n";
	lyxerr << "inset_id: " << inset_id << "\n";

	// Set the right(new) inset-owner of the paragraph if there is any.
	if (!undo.pars.empty()) {
		Inset * in = 0;
		if (before != null)
			in = before->inInset();
		else if (inset_id >= 0)
			in = bv->buffer()->getInsetFromID(inset_id);
		for (size_t i = 0, n = undo.pars.size(); i < n; ++i)
			undo.pars[i]->setInsetOwner(in);
	}


	// quick hack to make the common case work
	if (undo.pars.size() == 1 && boost::next(first) == after) {
	//	first = *undo.pars[0];
		lyxerr << "could use special case...\n";
	}

#if 0
	// remove stuff between first and after
	plist->erase(first, after);
	// re-create first
	if (before == null) {
		// if there's no before take the beginning of parlist.
		first = plist->begin();
	} else {
		first = *before;
		++first;
	}

#endif

	// inset saved paragraphs
	for (size_t i = 0, n = undo.pars.size(); i < n; ++i) {
		lyxerr << " inserting par " << undo.pars[i]->id() << "\n";
		lyxerr << " inserting plist: " << plist << "\n";
		//plist->insert(first, new Paragraph(*undo.pars[i], true));
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
	}

	// Set the cursor for redoing
	// if we have a par before the first.
	if (before != null) {
		Inset * it = before->inInset();
		if (it)
			it->getLyXText(bv)->setCursorIntern(*before, 0);
		else
			bv->text->setCursorIntern(*before, 0);
	}

	UpdatableInset * it = 0;
	if (first != plist->begin())
		it = static_cast<UpdatableInset*>(first->inInset());

	LyXText * text = it ? it->getLyXText(bv) : bv->text;

	text->redoParagraphs(text->cursor, plist->end());

	ParIterator tmppar = bv->buffer()->getParFromID(inset_id);

	if (tmppar != null) {
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
	}

	if (it) {
		bv->fitCursor();
		bv->updateInset(it);
		bv->text->setCursorIntern(bv->text->cursor.par(),
					  bv->text->cursor.pos());
	}


	finishUndo();
	bv->text->postPaint(0);

	return true;
}


bool createUndo(BufferView * bv, Undo::undo_kind kind,
	ParagraphList::iterator itfirst, ParagraphList::iterator itlast,
	shared_ptr<Undo> & u)
{
	Buffer * b = bv->buffer();

	ParIterator null    = b->par_iterator_end();
	ParIterator prev    = null;
	ParIterator before  = null;
	ParIterator first   = null;
	ParIterator last    = null;
	ParIterator behind  = null;

	int const first_id  = itfirst->id();
	int const last_id   = itlast->id();

	for (ParIterator it = b->par_iterator_begin(); it != null; ++it) {
		if ((*it)->id() == first_id) {
			first = it;
			before = prev;
		}
		if ((*it)->id() == last_id) {
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

	lyxerr << "\nbefore_id: " << before_id << "\n";
	lyxerr << "first_id:  " << first_id  << "\n";
	lyxerr << "last_id:   " << last_id   << "\n";
	lyxerr << "behind_id: " << behind_id << "\n";
	lyxerr << "inset_id:  " << inset_id  << "\n";

	ParagraphList * plist = 0;
	if (first != null)
		plist = &first.plist();
	else if (behind != null)
		plist = &behind.plist();
	else if (!plist) {
		lyxerr << "plist from buffer (should this happen?)\n";
		plist = &b->paragraphs;
	}

	// Undo::EDIT and Undo::FINISH are
	// always finished. (no overlapping there)
	// overlapping only with insert and delete inside one paragraph:
	// Nobody wants all removed  character
	// appear one by one when undoing.
	// EDIT is special since only layout information, not the
	// contents of a paragaph are stored.
	if (!undo_finished && (kind != Undo::EDIT) && (kind != Undo::FINISH)) {
		// Check whether storing is needed.
		if (!b->undostack.empty() &&
		    b->undostack.top()->kind == kind &&
		    b->undostack.top()->number_of_before_par == before_id &&
		    b->undostack.top()->number_of_behind_par == behind_id) {
			// No undo needed.
			return false;
		}
	}

	// Create a new Undo.
#warning FIXME Why is this a vector and not a ParagraphList?
	std::vector<Paragraph *> undo_pars;

	for (ParagraphList::iterator it = *first; it != *last; ++it) {
		int id = it->id();
		Paragraph * tmp = new Paragraph(*it);
		tmp->id(id);
		undo_pars.push_back(tmp);
	}

	int const id = last->id();
	Paragraph * tmp = new Paragraph(**last);
	tmp->id(id);
	undo_pars.push_back(tmp);

	// A memory optimization: Just store the layout
	// information when only edit.
#warning Waste...
	//if (kind == Undo::EDIT)
	//	for (size_t i = 0, n = undo_pars.size(); i < n; ++i)
	//		undo_pars[i]->clearContents();

	int cursor_par = undoCursor(bv).par()->id();
	int cursor_pos = undoCursor(bv).pos();

	u.reset(new Undo(kind, inset_id,
		before_id, behind_id, cursor_par, cursor_pos, undo_pars));

	undo_finished = false;
	return true;
}


// Returns false if no undo possible.
bool textUndoOrRedo(BufferView * bv,
	limited_stack<boost::shared_ptr<Undo> > & stack,
		    limited_stack<boost::shared_ptr<Undo> > & /*otherstack*/)
{
	//Buffer * b = bv->buffer();

	if (stack.empty()) {
		finishNoUndo(bv);
		return false;
	}

	shared_ptr<Undo> undo = stack.top();
	stack.pop();
	finishUndo();

	if (!undo_frozen) {
/*
		ParIterator p = b->getParFromID(undo->number_of_before_par);
		bool ok = false;
		ParagraphList::iterator first;
		// default constructed?
		ParIterator const end = b->par_iterator_end();
		if (p != end) {
			first = p.par();
			if (first->next())
				first = first->next();
		} else
			first = undoParagraphs(bv, undo->number_of_inset_id)->begin();
		if (first) {
			shared_ptr<Undo> u;
			ParIterator behind = b->getParFromID(undo->number_of_behind_par);
			if (createUndo(bv, undo->kind, first, behind.par(), u))
				otherstack.push(u);
		}
*/
	}

	// Now we can unlock the inset for saftey because the inset
	// pointer could be changed during the undo-function. Anyway
	// if needed we have to lock the right inset/position if this
	// is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool const ret = textHandleUndo(bv, *undo.get());
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
#warning DISABLED
	//return;
	if (!undo_frozen) {
		shared_ptr<Undo> u;
		if (createUndo(bv, kind, first, last, u))
			bv->buffer()->undostack.push(u);
		bv->buffer()->redostack.clear();
	}
}


void setCursorParUndo(BufferView * bv)
{
	setUndo(bv, Undo::FINISH, bv->text->cursor.par());
}
