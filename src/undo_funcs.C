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
Paragraph * firstUndoParagraph(BufferView * bv, int inset_id)
{
	Inset * inset = bv->buffer()->getInsetFromID(inset_id);
	if (inset) {
		Paragraph * result = inset->getFirstParagraph(0);
		if (result)
			return result;
	}
	return &*bv->text->ownerParagraphs().begin();
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

	Paragraph * const before = &*b->getParFromID(undo.number_of_before_par);
	Paragraph * const behind = &*b->getParFromID(undo.number_of_behind_par);

	// If there's no before take the beginning
	// of the document for redoing.
	if (!before) {
		LyXText * t = bv->text;
		int num = undo.number_of_inset_id;
		if (undo.number_of_inset_id >= 0) {
			Inset * in = bv->buffer()->getInsetFromID(num);
			if (in) {
				t = in->getLyXText(bv);
			} else {
				num = -1;
			}
		}
		t->setCursorIntern(firstUndoParagraph(bv, num), 0);
	}

	// Replace the paragraphs with the undo informations.

	Paragraph * undopar = undo.pars.empty() ? 0 : undo.pars[0];
	// Otherwise the undo destructor would
	// delete the paragraph.
	undo.pars.resize(0);

	// Get last undo par and set the right(new) inset-owner of the
	// paragraph if there is any. This is not needed if we don't
	// have a paragraph before because then in is automatically
	// done in the function which assigns the first paragraph to
	// an InsetText. (Jug)
	Paragraph * lastundopar = undopar;
	if (lastundopar) {
		Inset * in = 0;
		if (before)
			in = before->inInset();
		else if (undo.number_of_inset_id >= 0)
			in = bv->buffer()->getInsetFromID(undo.number_of_inset_id);
		lastundopar->setInsetOwner(in);
		while (lastundopar->next()) {
			lastundopar = lastundopar->next();
			lastundopar->setInsetOwner(in);
		}
	}

	vector<Paragraph *> deletelist;

	// Now add old paragraphs to be deleted.
	if (before != behind || (!behind && !before)) {
		Paragraph * deletepar;
		if (before)
			deletepar = before->next();
		else
			deletepar = firstUndoParagraph(bv, undo.number_of_inset_id);
		Paragraph * tmppar2 = undopar;
		while (deletepar && deletepar != behind) {
			deletelist.push_back(deletepar);
			Paragraph * tmppar = deletepar;
			deletepar = deletepar->next();

			// A memory optimization for edit:
			// Only layout information
			// is stored in the undo. So restore
			// the text informations.
			if (undo.kind == Undo::EDIT) {
				tmppar2->setContentsFromPar(*tmppar);
				tmppar2 = tmppar2->next();
			}
		}
	}

	// The order here is VERY IMPORTANT. We have to set the right
	// next/prev pointer in the paragraphs so that a rebuild of
	// the LyXText works!!!

	// Thread the end of the undo onto the par in front if any.
	if (lastundopar) {
		lastundopar->next(behind);
		if (behind)
			behind->previous(lastundopar);
	}

	// Put the new stuff in the list if there is one.
	if (undopar) {
		undopar->previous(before);
		if (before)
			before->next(undopar);
		else {
			int id = firstUndoParagraph(bv, undo.number_of_inset_id)->id();
			Paragraph * op = &*bv->buffer()->getParFromID(id);
			if (op && op->inInset()) {
				static_cast<InsetText*>(op->inInset())->paragraph(undopar);
			} else {
				bv->buffer()->paragraphs.set(undopar);
			}
		}
	} else {
		// We enter here on DELETE undo operations where we
		// have to substitue the second paragraph with the
		// first if the removed one is the first.
		if (!before && behind) {
			int id = firstUndoParagraph(bv, undo.number_of_inset_id)->id();
			Paragraph * op = &*bv->buffer()->getParFromID(id);
			if (op && op->inInset()) {
				static_cast<InsetText*>(op->inInset())->paragraph(behind);
			} else {
				bv->buffer()->paragraphs.set(behind);
			}

			undopar = behind;
		}
	}


	// Set the cursor for redoing.
	// If we have a par before the undopar.
	if (before) {
		Inset * it = before->inInset();
		if (it)
			it->getLyXText(bv)->setCursorIntern(before, 0);
		else
			bv->text->setCursorIntern(before, 0);
	}

// we are not ready for this we cannot set the cursor for a paragraph
// which is not already in a row of LyXText!!!
#if 0
	else { // otherwise this is the first one and we start here
		Inset * it = undopar->inInset();
		if (it)
			it->getLyXText(bv)->setCursorIntern(bv, undopar, 0);
		else
			bv->text->setCursorIntern(bv, undopar, 0);
	}
#endif

	Paragraph * endpar = 0;

	// Calculate the endpar for redoing the paragraphs.
	if (behind)
		endpar = behind->next();

	UpdatableInset * it = 0;
	if (undopar)
		it = static_cast<UpdatableInset*>(undopar->inInset());
	if (it) {
		it->getLyXText(bv)->redoParagraphs(
						   it->getLyXText(bv)->cursor,
						   endpar);
		Paragraph * tmppar =
			&*bv->buffer()->getParFromID(undo.number_of_cursor_par);
		if (tmppar) {
			it = static_cast<UpdatableInset*>(tmppar->inInset());
			LyXText * t;
			if (it) {
				it->edit(bv);
				t = it->getLyXText(bv);
			} else {
				t = bv->text;
			}
			t->setCursorIntern(tmppar, undo.cursor_pos);
			// Clear any selection and set the selection
			// cursor for an evt. new selection.
			t->clearSelection();
			t->selection.cursor = t->cursor;
			t->updateCounters();
			bv->fitCursor();
		}
		bv->updateInset(it);
		bv->text->setCursorIntern(bv->text->cursor.par(),
					  bv->text->cursor.pos());
	} else {
		bv->text->redoParagraphs(bv->text->cursor, endpar);
		Paragraph * tmppar =
			&*bv->buffer()->getParFromID(undo.number_of_cursor_par);
		if (tmppar) {
			LyXText * t;
			Inset * it = tmppar->inInset();
			if (it) {
				it->edit(bv);
				t = it->getLyXText(bv);
			} else {
				t = bv->text;
			}
			t->setCursorIntern(tmppar, undo.cursor_pos);
			// Clear any selection and set the selection
			// cursor for an evt. new selection.
			t->clearSelection();
			t->selection.cursor = t->cursor;
			t->updateCounters();
		}
	}

	// And here it's safe enough to delete all removed paragraphs.
	vector<Paragraph *>::iterator pit = deletelist.begin();
	for(; pit != deletelist.end(); ++pit) {
		(*pit)->previous(0);
		(*pit)->next(0);
		delete (*pit);
	}

	finishUndo();
	bv->text->postPaint(0);
	return true;
}


bool createUndo(BufferView * bv, Undo::undo_kind kind,
	ParagraphList::iterator itfirst, ParagraphList::iterator itbehind,
	shared_ptr<Undo> & u)
{
	Paragraph * first = &*itfirst;
	Paragraph * behind = &*itbehind;
	lyx::Assert(first);

	int before_number = -1;
	int behind_number = -1;
	int inset_id = -1;

	if (first->previous())
		before_number = first->previous()->id();
	if (behind)
		behind_number = behind->id();
	if (first->inInset())
		inset_id = first->inInset()->id();

	Buffer * b = bv->buffer();

	// Undo::EDIT  and Undo::FINISH are
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
		    b->undostack.top()->number_of_before_par == before_number &&
		    b->undostack.top()->number_of_behind_par == behind_number) {
			// No undo needed.
			return false;
		}
	}

	// Create a new Undo.
	std::vector<Paragraph *> undo_pars;

	Paragraph * start = first;
	Paragraph * end = 0;

	if (behind)
		end = const_cast<Paragraph*>(behind->previous());
	else {
		end = start;
		while (end->next())
			end = end->next();
	}
	if (start && end && (start != end->next()) &&
	    ((before_number != behind_number) ||
		 ((before_number < 0) && (behind_number < 0))))
	{
		Paragraph * tmppar = start;
		undo_pars.push_back(new Paragraph(*tmppar, true));

		// A memory optimization: Just store the layout
		// information when only edit.
		if (kind == Undo::EDIT) {
			undo_pars.back()->clearContents();
		}

		while (tmppar != end && tmppar->next()) {
			tmppar = tmppar->next();
			undo_pars.push_back(new Paragraph(*tmppar, true));
			size_t const n = undo_pars.size();
			undo_pars[n - 2]->next(undo_pars[n - 1]);
			undo_pars[n - 1]->previous(undo_pars[n - 2]);
			// a memory optimization: Just store the layout
			// information when only edit
			if (kind == Undo::EDIT) {
				undo_pars.back()->clearContents();
			}
		}
		undo_pars.back()->next(0);
	}

	int cursor_par = undoCursor(bv).par()->id();
	int cursor_pos = undoCursor(bv).pos();

	u.reset(new Undo(kind, inset_id,
		before_number, behind_number,
		cursor_par, cursor_pos, undo_pars));

	undo_finished = false;
	return true;
}


// Returns false if no undo possible.
bool textUndoOrRedo(BufferView * bv,
	limited_stack<boost::shared_ptr<Undo> > & stack,
	limited_stack<boost::shared_ptr<Undo> > & otherstack)
{
	Buffer * b = bv->buffer();

	if (stack.empty()) {
		finishNoUndo(bv);
		return false;
	}

	shared_ptr<Undo> undo = stack.top();
	stack.pop();
	finishUndo();

	if (!undo_frozen) {
		Paragraph * first = &*b->getParFromID(undo->number_of_before_par);
		if (first && first->next())
			first = first->next();
		else if (!first)
			first = firstUndoParagraph(bv, undo->number_of_inset_id);
		if (first) {
			shared_ptr<Undo> u;
			if (createUndo(bv, undo->kind, first,
				             b->getParFromID(undo->number_of_behind_par), u))
				otherstack.push(u);
		}
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
	     ParagraphList::iterator first, ParagraphList::iterator behind)
{
	if (!undo_frozen) {
		shared_ptr<Undo> u;
		if (createUndo(bv, kind, first, behind, u))
			bv->buffer()->undostack.push(u);
		bv->buffer()->redostack.clear();
	}
}


void setRedo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first, ParagraphList::iterator behind)
{
	shared_ptr<Undo> u;
	if (createUndo(bv, kind, first, behind, u))
		bv->buffer()->redostack.push(u);
}


void setCursorParUndo(BufferView * bv)
{
	setUndo(bv, Undo::FINISH, bv->text->cursor.par(),
		boost::next(bv->text->cursor.par()));
}
