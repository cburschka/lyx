/**
 * \file undo.C
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

#include "undo.h"

#include "buffer.h"
#include "debug.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "iterators.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/updatableinset.h"
#include "insets/insettext.h"

#include <iostream>

using std::endl;
using lyx::paroffset_type;


/// The flag used by finishUndo().
bool undo_finished;

/// Whether actions are not added to the undo stacks.
bool undo_frozen;

Undo::Undo(undo_kind kind_arg, int text_arg,
	   int first, int last,
	   int cursor, int cursor_pos_arg,
	   ParagraphList const & par)
	:
		kind(kind_arg),
		text(text_arg),
		first_par_offset(first),
		last_par_offset(last),
		cursor_par_offset(cursor),
		cursor_pos(cursor_pos_arg),
		pars(par)
{}


std::ostream & operator<<(std::ostream & os, Undo const & undo)
{
	return os << " text: " << undo.text
		<< " first: " << undo.first_par_offset
		<< " last: " << undo.last_par_offset
		<< " cursor: " << undo.cursor_par_offset
		<< "/" << undo.cursor_pos;
}


namespace {

void recordUndo(Undo::undo_kind kind,
	LyXText * text, paroffset_type firstpar, paroffset_type lastpar,
	limited_stack<Undo> & stack)
{
	Buffer * buf = text->bv()->buffer();

	ParagraphList & plist = text->ownerParagraphs();
	ParagraphList::iterator first = plist.begin();
	advance(first, firstpar);
	ParagraphList::iterator last = plist.begin();
	advance(last, lastpar);

	// try to find the appropriate list by counting the
	// texts from buffer begin
	ParIterator null = buf->par_iterator_end();

	int tcount = 0;
	// it.text() returns 0 for outermost text.
	if (text != text->bv()->text)
		for (ParIterator it = buf->par_iterator_begin(); it != null; ++it, ++tcount)
			if (it.text() == text)
				break;
		
	// and calculate a stable reference to them
	int const first_offset = firstpar;
	int const last_offset = plist.size() - lastpar;

	// Undo::ATOMIC are always recorded (no overlapping there).
	// overlapping only with insert and delete inside one paragraph:
	// nobody wants all removed character appear one by one when undoing.
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
	int const cursor_offset = text->cursor.par();

	// make and push the Undo entry
	stack.push(Undo(kind, tcount,
		first_offset, last_offset,
		cursor_offset, text->cursor.pos(),
		ParagraphList()));
	lyxerr << "undo record: " << stack.top() << endl;

	// record the relevant paragraphs
	ParagraphList & undo_pars = stack.top().pars;

	for (ParagraphList::iterator it = first; it != last; ++it) {
		undo_pars.push_back(*it);
		undo_pars.back().id(it->id());
	}
	undo_pars.push_back(*last);
	undo_pars.back().id(last->id());

	// and make sure that next time, we should be combining if possible
	undo_finished = false;
}


// Returns false if no undo possible.
bool performUndoOrRedo(BufferView * bv, Undo const & undo)
{
	lyxerr << "undo, performing: " << undo << endl;
	Buffer * buf = bv->buffer();
	ParIterator plit = buf->par_iterator_begin();
	ParIterator null = buf->par_iterator_end();

	int tcount = undo.text;
	for ( ; tcount && plit != null; ++plit, --tcount)
		;

	LyXText * text = plit.text();
	if (!text)
		text = bv->text;
	ParagraphList & plist = text->ownerParagraphs();

	// remove new stuff between first and last
	{
		ParagraphList::iterator first = plist.begin();
		advance(first, undo.first_par_offset);
		ParagraphList::iterator last = plist.begin();
		advance(last, plist.size() - undo.last_par_offset);
		plist.erase(first, ++last);
	}

	// re-insert old stuff instead
	if (plist.empty()) {
		plist.assign(undo.pars.begin(), undo.pars.end());
	} else {
		ParagraphList::iterator first = plist.begin();
		advance(first, undo.first_par_offset);
		plist.insert(first, undo.pars.begin(), undo.pars.end());
	}

	// set cursor
	lyxerr <<   "undo, text: " << text << " inset: " << plit.inset() << endl;
	InsetOld * inset = plit.inset();
	if (inset) {
		// Magic needed to cope with inset locking
		FuncRequest cmd(bv, LFUN_INSET_EDIT, "left");
		inset->localDispatch(cmd);
	}

	// set cursor again to force the position to be the right one
	text->setCursorIntern(undo.cursor_par_offset, undo.cursor_pos);

	// clear any selection
	text->clearSelection();
	text->selection.cursor = text->cursor;
	text->updateCounters();

	// rebreak the entire document
	bv->text->fullRebreak();

	finishUndo();
	return true;
}


// Returns false if no undo possible.
bool textUndoOrRedo(BufferView * bv,
	limited_stack<Undo> & stack, limited_stack<Undo> & otherstack)
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
		otherstack.top().cursor_pos = text
		lyxerr << " undo other: " << otherstack.top() << endl;
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


void recordUndo(Undo::undo_kind kind,
	LyXText const * text, paroffset_type first, paroffset_type last)
{
	if (!undo_frozen) {
		Buffer * buf = text->bv()->buffer();
		recordUndo(kind, const_cast<LyXText *>(text),
			first, last, buf->undostack());
		buf->redostack().clear();
	}
}


void recordUndo(Undo::undo_kind kind, LyXText const * text, paroffset_type par)
{
	recordUndo(kind, text, par, par);
}


void recordUndo(BufferView * bv, Undo::undo_kind kind)
{
	recordUndo(kind, bv->text, bv->text->cursor.par());
}
