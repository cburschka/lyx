/**
 * \file undo_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "undo_funcs.h"

#include "buffer.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "iterators.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/updatableinset.h"


/// The flag used by FinishUndo().
bool undo_finished;

/// Whether actions are not added to the undo stacks.
bool undo_frozen;

namespace {


void recordUndo(BufferView * bv, Undo::undo_kind kind,
	ParagraphList::iterator first, ParagraphList::iterator last,
	limited_stack<Undo> & stack)
{
	Buffer * buf = bv->buffer();

	// First, record inset id, if cursor is in one
	UpdatableInset * inset = first->inInset();
	LyXText * text = inset ? inset->getLyXText(bv) : bv->text;
	int const inset_id = inset ? inset->id() : -1;

	// We simply record the entire outer paragraphs
	ParagraphList & plist = buf->paragraphs();
	ParIterator null = buf->par_iterator_end();

	// First, identify the outer paragraphs
	for (ParIterator it = buf->par_iterator_begin(); it != null; ++it) {
		if (it->id() == first->id())
			first = it.outerPar();
		if (it->id() == last->id())
			last = it.outerPar();
	}

	// And calculate a stable reference to them
	int const first_offset = std::distance(plist.begin(), first);
	int const last_offset = std::distance(last, plist.end());

	// Undo::ATOMIC are always recorded (no overlapping there).

	// Overlapping only with insert and delete inside one paragraph:
	// Nobody wants all removed character appear one by one when undoing.
	if (! undo_finished && kind != Undo::ATOMIC) {
		// Check whether storing is needed.
		if (! buf->undostack().empty() 
		    && buf->undostack().top().kind == kind 
		    && buf->undostack().top().first_par_offset == first_offset
		    && buf->undostack().top().last_par_offset == last_offset) {
			// No additonal undo recording needed -
			// effectively, we combine undo recordings to one.
			return;
		}
	}

	// Record the cursor position in a stable way.
	int const cursor_offset = std::distance
		(text->ownerParagraphs().begin(), text->cursor.par());

	// Make and push the Undo entry
	stack.push(Undo(kind, inset_id,
		first_offset, last_offset,
		cursor_offset, text->cursor.pos(),
		ParagraphList()));

	// Record the relevant paragraphs
	ParagraphList & undo_pars = stack.top().pars;

	for (ParagraphList::iterator it = first; it != last; ++it) {
		undo_pars.push_back(*it);
		undo_pars.back().id(it->id());
	}
	undo_pars.push_back(*last);
	undo_pars.back().id(last->id());

	// And make sure that next time, we should be combining if possible
	undo_finished = false;
}


// Returns false if no undo possible.
bool performUndoOrRedo(BufferView * bv, Undo & undo)
{
	Buffer * buf = bv->buffer();
	ParagraphList & plist = buf->paragraphs();

	// Remove new stuff between first and last
	{
		ParagraphList::iterator first = plist.begin();
		advance(first, undo.first_par_offset);
		ParagraphList::iterator last = plist.begin();
		advance(last, plist.size() - undo.last_par_offset);
		plist.erase(first, ++last);
	}
		
	// Re-insert old stuff instead
	{
		if (plist.empty()) {
			plist.assign(undo.pars.begin(), undo.pars.end());
		} else {
			ParagraphList::iterator first = plist.begin();
			advance(first, undo.first_par_offset);
			plist.insert(first, undo.pars.begin(), undo.pars.end());
		}
	}

	// Rebreak the entire document
	bv->text->fullRebreak();

	// set cursor
	{
		// Get a hold of the inset for the cursor, if relevant
		UpdatableInset * inset =
			static_cast<UpdatableInset *>(
				buf->getInsetFromID(undo.inset_id));

		LyXText * text = inset ? inset->getLyXText(bv) : bv->text;
		ParagraphList::iterator cursor = text->ownerParagraphs().begin();
		advance(cursor, undo.cursor_par_offset);
		text->setCursorIntern(cursor, undo.cursor_pos);

		if (inset) {
			// Magic needed to update inset internal state
			FuncRequest cmd(bv, LFUN_INSET_EDIT, "left");
			inset->localDispatch(cmd);
		}

		// set cursor again to force the position to be the right one
		text->setCursorIntern(cursor, undo.cursor_pos);

		// Clear any selection and set the selection
		// cursor for any new selection.
		text->clearSelection();
		text->selection.cursor = text->cursor;
		text->updateCounters();
	}

	finishUndo();
	return true;
}


// Returns false if no undo possible.
bool textUndoOrRedo(BufferView * bv,
	limited_stack<Undo> & stack,
	limited_stack<Undo> & otherstack)
{
	if (stack.empty()) {
		/*
		 * Finish the undo operation in the case there was no entry
		 * on the stack to perform.
		 */
		freezeUndo();
		bv->unlockInset(bv->theLockingInset());
		finishUndo();
		unFreezeUndo();
		return false;
	}

	Undo undo = stack.top();
	stack.pop();
	finishUndo();

	if (!undo_frozen) {
		otherstack.push(undo);
		otherstack.top().pars.clear();
		Buffer * buf = bv->buffer();
		ParagraphList & plist = buf->paragraphs();
		if (undo.first_par_offset + undo.last_par_offset <= int(plist.size())) {
			ParagraphList::iterator first = plist.begin();
			advance(first, undo.first_par_offset);
			ParagraphList::iterator last = plist.begin();
			advance(last, plist.size() - undo.last_par_offset + 1);
			otherstack.top().pars.insert(otherstack.top().pars.begin(), first, last);
		}
	}

	// Now we can unlock the inset for safety because the inset
	// pointer could be changed during the undo-function. Anyway
	// if needed we have to lock the right inset/position if this
	// is requested.
	freezeUndo();
	bv->unlockInset(bv->theLockingInset());
	bool const ret = performUndoOrRedo(bv, undo);
	unFreezeUndo();
	return ret;
}

} // namespace anon


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


void finishUndo()
{
	// Makes sure the next operation will be stored.
	undo_finished = true;
}


bool textUndo(BufferView * bv)
{
	return textUndoOrRedo(bv, bv->buffer()->undostack(),
			      bv->buffer()->redostack());
}


bool textRedo(BufferView * bv)
{
	return textUndoOrRedo(bv, bv->buffer()->redostack(),
			      bv->buffer()->undostack());
}


void recordUndo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first, ParagraphList::iterator last)
{
	if (!undo_frozen) {
		recordUndo(bv, kind, first, last, bv->buffer()->undostack());
		bv->buffer()->redostack().clear();
	}
}


void recordUndo(BufferView * bv, Undo::undo_kind kind,
	     ParagraphList::iterator first)
{
	recordUndo(bv, kind, first, first);
}


void recordUndo(BufferView * bv, Undo::undo_kind kind)
{
	recordUndo(bv, kind, bv->text->cursor.par());
}

