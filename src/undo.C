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
#include "iterators.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/updatableinset.h" // for dynamic_cast<UpdatableInset *>

using lyx::paroffset_type;


/// The flag used by finishUndo().
bool undo_finished;

/// Whether actions are not added to the undo stacks.
bool undo_frozen;

Undo::Undo(undo_kind kind_, int text_, int index_,
	   int first_par_, int end_par_, int cursor_par_, int cursor_pos_)
	:
		kind(kind_),
		text(text_),
		index(index_),
		first_par(first_par_),
		end_par(end_par_),
		cursor_par(cursor_par_),
		cursor_pos(cursor_pos_)
{}


namespace {

std::ostream & operator<<(std::ostream & os, Undo const & undo)
{
	return os << " text: " << undo.text
		<< " index: " << undo.index
		<< " first: " << undo.first_par
		<< " from end: " << undo.end_par
		<< " cursor: " << undo.cursor_par
		<< "/" << undo.cursor_pos;
}


// translates LyXText pointer into offset count from document begin
ParIterator text2pit(BufferView * bv, LyXText * text, int & tcount)
{
	tcount = 0;
	Buffer * buf = text->bv()->buffer();
	ParIterator pit = buf->par_iterator_begin();
	ParIterator end = buf->par_iterator_end();

	for ( ; pit != end; ++pit, ++tcount)
		if (pit.text(bv) == text)
			return pit;
	lyxerr << "undo: should not happen" << std::endl;
	return end;
}


// translates offset from buffer begin to ParIterator
ParIterator num2pit(BufferView * bv, int num)
{
	Buffer * buf = bv->buffer();
	ParIterator pit = buf->par_iterator_begin();
	ParIterator end = buf->par_iterator_end();

	for ( ; num && pit != end; ++pit, --num)
		;

	if (pit != end)
		return pit;

	// don't crash early...
	lyxerr << "undo: num2pit: num: " << num << std::endl;
	BOOST_ASSERT(false);
	return buf->par_iterator_begin();
}


void recordUndo(Undo::undo_kind kind,
	LyXText * text, paroffset_type first_par, paroffset_type last_par,
	limited_stack<Undo> & stack)
{
	Buffer * buf = text->bv()->buffer();

	int const end_par = text->ownerParagraphs().size() - last_par;

	// Undo::ATOMIC are always recorded (no overlapping there).
	// overlapping only with insert and delete inside one paragraph:
	// nobody wants all removed character appear one by one when undoing.
	if (!undo_finished && kind != Undo::ATOMIC) {
		// Check whether storing is needed.
		if (!buf->undostack().empty()
		    && buf->undostack().top().kind == kind
		    && buf->undostack().top().first_par == first_par
		    && buf->undostack().top().end_par == end_par) {
			// No additonal undo recording needed -
			// effectively, we combine undo recordings to one.
			return;
		}
	}

	// make and push the Undo entry
	int textnum;
	ParIterator pit = text2pit(text->bv(), text, textnum);
	stack.push(Undo(kind, textnum, pit.index(),
		first_par, end_par, text->cursor.par(), text->cursor.pos()));
	lyxerr << "undo record: " << stack.top() << std::endl;

	// record the relevant paragraphs
	ParagraphList & undo_pars = stack.top().pars;

	ParagraphList & plist = text->ownerParagraphs();
	ParagraphList::iterator first = plist.begin();
	advance(first, first_par);
	ParagraphList::iterator last = plist.begin();
	advance(last, last_par);

	for (ParagraphList::iterator it = first; it != last; ++it)
		undo_pars.push_back(*it);
	undo_pars.push_back(*last);

	// and make sure that next time, we should be combining if possible
	undo_finished = false;
}


// returns false if no undo possible
bool performUndoOrRedo(BufferView * bv, Undo const & undo)
{
	lyxerr << "undo, performing: " << undo << std::endl;
	ParIterator pit = num2pit(bv, undo.text);
	LyXText * text = pit.text(bv);
	ParagraphList & plist = text->ownerParagraphs();

	// remove new stuff between first and last
	{
		ParagraphList::iterator first = plist.begin();
		advance(first, undo.first_par);
		ParagraphList::iterator last = plist.begin();
		advance(last, plist.size() - undo.end_par);
		plist.erase(first, ++last);
	}

	// re-insert old stuff instead
	if (plist.empty()) {
		plist.assign(undo.pars.begin(), undo.pars.end());
	} else {
		ParagraphList::iterator first = plist.begin();
		advance(first, undo.first_par);
		plist.insert(first, undo.pars.begin(), undo.pars.end());
	}

	// set cursor
	lyxerr <<   "undo, text: " << undo.text
	       << " inset: " << pit.inset()
	       << " index: " << undo.index
	       << " par: " << undo.cursor_par
	       << " pos: " << undo.cursor_pos
	       << std::endl;

	// set cursor again to force the position to be the right one
	text->cursor.par(undo.cursor_par);
	text->cursor.pos(undo.cursor_pos);

	// clear any selection
	text->clearSelection();
	text->selection.cursor = text->cursor;
	text->updateCounters();

	// rebreak the entire lyxtext
	bv->text->fullRebreak();

	pit.lockPath(bv);
	
	finishUndo();
	return true;
}


// returns false if no undo possible
bool textUndoOrRedo(BufferView * bv,
	limited_stack<Undo> & stack, limited_stack<Undo> & otherstack)
{
	if (stack.empty()) {
		// nothing to do
		finishUndo();
		return false;
	}

	Undo undo = stack.top();
	stack.pop();
	finishUndo();

	if (!undo_frozen) {
		otherstack.push(undo);
		otherstack.top().pars.clear();
		ParIterator pit = num2pit(bv, undo.text);
		ParagraphList & plist = pit.plist();
		if (undo.first_par + undo.end_par <= int(plist.size())) {
			ParagraphList::iterator first = plist.begin();
			advance(first, undo.first_par);
			ParagraphList::iterator last = plist.begin();
			advance(last, plist.size() - undo.end_par + 1);
			otherstack.top().pars.insert(otherstack.top().pars.begin(), first, last);
		}
		LyXText * text = pit.text(bv);
		otherstack.top().cursor_pos = text->cursor.pos();
		otherstack.top().cursor_par = text->cursor.par();
		lyxerr << " undo other: " << otherstack.top() << std::endl;
	}

	freezeUndo();
	bool const ret = performUndoOrRedo(bv, undo);
	unFreezeUndo();
	return ret;
}

} // namespace anon


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


void finishUndo()
{
	// makes sure the next operation will be stored
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
	if (undo_frozen)
		return;
	Buffer * buf = text->bv()->buffer();
	recordUndo(kind, const_cast<LyXText *>(text), first, last, buf->undostack());
	buf->redostack().clear();
}


void recordUndo(Undo::undo_kind kind, LyXText const * text, paroffset_type par)
{
	recordUndo(kind, text, par, par);
}


void recordUndo(BufferView * bv, Undo::undo_kind kind)
{
	recordUndo(kind, bv->text, bv->text->cursor.par());
}
