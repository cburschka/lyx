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

#include "undo_funcs.h"
#include "lyxtext.h"
#include "BufferView.h"
#include "buffer.h"
#include "insets/inset.h"
#include "debug.h"
#include "support/LAssert.h"

#include "iterators.h"

#include <vector>

using std::vector;
using boost::shared_ptr;


/// the flag used by FinishUndo();
bool undo_finished;
/// whether actions are not added to the undo stacks
bool undo_frozen;

namespace {

/// utility to return the cursor
LyXCursor const & undoCursor(BufferView * bv)
{
	if (bv->theLockingInset())
		return bv->theLockingInset()->cursor(bv);
	return bv->text->cursor;
}

/**
 * returns a pointer to the very first Paragraph depending of where we are
 * so it will return the first paragraph of the buffer or the first paragraph
 * of the textinset we're in.
 */
Paragraph * firstUndoParagraph(BufferView * bv, int inset_id)
{
	Inset * inset = bv->buffer()->getInsetFromID(inset_id);
	if (inset) {
		Paragraph * result = inset->getFirstParagraph(0);
		if (result)
			return result;
	}
	return bv->text->ownerParagraph();
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
	bv->text->status(bv, LyXText::NEED_MORE_REFRESH);
	unFreezeUndo();
}


// returns false if no undo possible
bool textHandleUndo(BufferView * bv, Undo & undo)
{
	Buffer * b = bv->buffer();

	Paragraph * before =
		b->getParFromID(undo.number_of_before_par);
	Paragraph * behind =
		b->getParFromID(undo.number_of_behind_par);
	Paragraph * tmppar;
	Paragraph * tmppar2;

	// if there's no before take the beginning
	// of the document for redoing
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
		t->setCursorIntern(bv, firstUndoParagraph(bv, num), 0);
	}

	// replace the paragraphs with the undo informations

	Paragraph * undopar = undo.par;
	undo.par = 0;	/* otherwise the undo destructor would
			   delete the paragraph */

	// get last undo par and set the right(new) inset-owner of the
	// paragraph if there is any. This is not needed if we don't have
	// a paragraph before because then in is automatically done in the
	// function which assigns the first paragraph to an InsetText. (Jug)
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

	Paragraph * deletepar;

	// now add old paragraphs to be deleted
	if (before != behind || (!behind && !before)) {
		if (before)
			deletepar = before->next();
		else
			deletepar = firstUndoParagraph(bv, undo.number_of_inset_id);
		tmppar2 = undopar;
		while (deletepar && deletepar != behind) {
			deletelist.push_back(deletepar);
			tmppar = deletepar;
			deletepar = deletepar->next();

			// a memory optimization for edit:
			// Only layout information
			// is stored in the undo. So restore
			// the text informations.
			if (undo.kind == Undo::EDIT) {
				tmppar2->setContentsFromPar(tmppar);
				tmppar2 = tmppar2->next();
			}
		}
	}

	// The order here is VERY IMPORTANT. We have to set the right
	// next/prev pointer in the paragraphs so that a rebuild of
	// the LyXText works!!!

	// thread the end of the undo onto the par in front if any
	if (lastundopar) {
		lastundopar->next(behind);
		if (behind)
			behind->previous(lastundopar);
	}

	// put the new stuff in the list if there is one
	if (undopar) {
		undopar->previous(before);
		if (before)
			before->next(undopar);
		else
			bv->text->ownerParagraph(firstUndoParagraph(bv, undo.number_of_inset_id)->id(),
						 undopar);

	} else {
		// We enter here on DELETE undo operations where we have to
		// substitue the second paragraph with the first if the removed
		// one is the first!
		if (!before && behind) {
			bv->text->ownerParagraph(firstUndoParagraph(bv, undo.number_of_inset_id)->id(),
						 behind);
			undopar = behind;
		}
	}


	// Set the cursor for redoing
	if (before) { // if we have a par before the undopar
		Inset * it = before->inInset();
		if (it)
			it->getLyXText(bv)->setCursorIntern(bv, before, 0);
		else
			bv->text->setCursorIntern(bv, before, 0);
	} else { // otherwise this is the first one and we start here
		Inset * it = undopar->inInset();
		if (it)
			it->getLyXText(bv)->setCursorIntern(bv, undopar, 0);
		else
			bv->text->setCursorIntern(bv, undopar, 0);
	}

	Paragraph * endpar = 0;
	// calculate the endpar for redoing the paragraphs.
	if (behind)
		endpar = behind->next();

	tmppar = bv->buffer()->getParFromID(undo.number_of_cursor_par);
	UpdatableInset* it = 0;
	if (undopar)
		it = static_cast<UpdatableInset*>(undopar->inInset());
	if (it) {
		it->getLyXText(bv)->redoParagraphs(bv,
						   it->getLyXText(bv)->cursor,
						   endpar);
		if (tmppar) {
			it = static_cast<UpdatableInset*>(tmppar->inInset());
			LyXText * t;
			if (it) {
				it->edit(bv);
				t = it->getLyXText(bv);
			} else {
				t = bv->text;
			}
			t->setCursorIntern(bv, tmppar, undo.cursor_pos);
			// clear any selection and set the selection cursor
			// for an evt. new selection.
			t->clearSelection();
			t->selection.cursor = t->cursor;
			t->updateCounters(bv);
			bv->fitCursor();
		}
		bv->updateInset(it, false);
		bv->text->setCursorIntern(bv, bv->text->cursor.par(),
					  bv->text->cursor.pos());
	} else {
		bv->text->redoParagraphs(bv, bv->text->cursor, endpar);
		if (tmppar) {
			LyXText * t;
			Inset * it = tmppar->inInset();
			if (it) {
				it->edit(bv);
				t = it->getLyXText(bv);
			} else {
				t = bv->text;
			}
			t->setCursorIntern(bv, tmppar, undo.cursor_pos);
			// clear any selection and set the selection cursor
			// for an evt. new selection.
			t->clearSelection();
			t->selection.cursor = t->cursor;
			t->updateCounters(bv);
		}
	}

	// And here it's safe enough to delete all removed paragraphs
	vector<Paragraph *>::iterator pit = deletelist.begin();
	if (pit != deletelist.end()) {
			for(;pit != deletelist.end(); ++pit) {
				(*pit)->previous(0);
				(*pit)->next(0);
				delete (*pit);
			}
		}

	finishUndo();
	bv->text->status(bv, LyXText::NEED_MORE_REFRESH);
	return true;
}


bool createUndo(BufferView * bv, Undo::undo_kind kind,
	Paragraph const * first, Paragraph const * behind, shared_ptr<Undo> & u)
{
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
		// check whether storing is needed
		if (!b->undostack.empty() &&
		    b->undostack.top()->kind == kind &&
		    b->undostack.top()->number_of_before_par == before_number &&
		    b->undostack.top()->number_of_behind_par == behind_number) {
			// no undo needed
			return false;
		}
	}

	// create a new Undo
	Paragraph * undopar;

	Paragraph * start = const_cast<Paragraph *>(first);
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
		Paragraph * tmppar2 = new Paragraph(*tmppar, true);

		// a memory optimization: Just store the layout information
		// when only edit
		if (kind == Undo::EDIT) {
			tmppar2->clearContents();
		}

		undopar = tmppar2;

		while (tmppar != end && tmppar->next()) {
			tmppar = tmppar->next();
			tmppar2->next(new Paragraph(*tmppar, true));
			// a memory optimization: Just store the layout
			// information when only edit
			if (kind == Undo::EDIT) {
				tmppar2->clearContents();
			}
			tmppar2->next()->previous(tmppar2);

			tmppar2 = tmppar2->next();
		}
		tmppar2->next(0);
	} else
		undopar = 0; // nothing to replace (undo of delete maybe)

	int cursor_par = undoCursor(bv).par()->id();
	int cursor_pos =  undoCursor(bv).pos();

	u.reset(new Undo(kind, inset_id,
		before_number, behind_number,
		cursor_par, cursor_pos, undopar));

	undo_finished = false;
	return true;
}

} // namespace anon

void finishUndo()
{
	// makes sure the next operation will be stored
	undo_finished = true;
}


void freezeUndo()
{
	// this is dangerous and for internal use only
	undo_frozen = true;
}


void unFreezeUndo()
{
	// this is dangerous and for internal use only
	undo_frozen = false;
}


// returns false if no undo possible
bool textUndo(BufferView * bv)
{
	Buffer * b = bv->buffer();

	if (b->undostack.empty()) {
		finishNoUndo(bv);
		return false;
	}

	shared_ptr<Undo> undo = b->undostack.top();
	b->undostack.pop();
	finishUndo();

	if (!undo_frozen) {
		Paragraph * first = b->getParFromID(undo->number_of_before_par);
		if (first && first->next())
			first = first->next();
		else if (!first)
			first = firstUndoParagraph(bv, undo->number_of_inset_id);
		if (first) {
			shared_ptr<Undo> u;
			if (createUndo(bv, undo->kind, first,
				b->getParFromID(undo->number_of_behind_par), u))
				b->redostack.push(u);
		}
	}

	// now we can unlock the inset for saftey because the inset pointer could
	// be changed during the undo-function. Anyway if needed we have to lock
	// the right inset/position if this is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool const ret = textHandleUndo(bv, *undo.get());
	unFreezeUndo();
	return ret;
}


// returns false if no redo possible
bool textRedo(BufferView * bv)
{
	Buffer * b = bv->buffer();

	if (b->redostack.empty()) {
		finishNoUndo(bv);
		return false;
	}

	shared_ptr<Undo> undo = b->redostack.top();
	b->redostack.pop();
	finishUndo();

	if (!undo_frozen) {
	Paragraph * first = bv->buffer()->getParFromID(undo->number_of_before_par);
		if (first && first->next())
			first = first->next();
		else if (!first)
			first = firstUndoParagraph(bv, undo->number_of_inset_id);
		if (first) {
			shared_ptr<Undo> u;
			if (createUndo(bv, undo->kind, first,
				bv->buffer()->getParFromID(undo->number_of_behind_par), u))
				bv->buffer()->undostack.push(u);
		}
	}

	// now we can unlock the inset for saftey because the inset pointer could
	// be changed during the undo-function. Anyway if needed we have to lock
	// the right inset/position if this is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool ret = textHandleUndo(bv, *undo.get());
	unFreezeUndo();
	return ret;
}


void setUndo(BufferView * bv, Undo::undo_kind kind,
	     Paragraph const * first, Paragraph const * behind)
{
	if (!undo_frozen) {
		shared_ptr<Undo> u;
		if (createUndo(bv, kind, first, behind, u))
			bv->buffer()->undostack.push(u);
		bv->buffer()->redostack.clear();
	}
}


void setRedo(BufferView * bv, Undo::undo_kind kind,
	     Paragraph const * first, Paragraph const * behind)
{
	shared_ptr<Undo> u;
	if (createUndo(bv, kind, first, behind, u))
		bv->buffer()->redostack.push(u);
}


void setCursorParUndo(BufferView * bv)
{
	setUndo(bv, Undo::FINISH, bv->text->cursor.par(),
		bv->text->cursor.par()->next());
}
