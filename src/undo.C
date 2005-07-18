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
#include "cursor.h"
#include "debug.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "mathed/math_support.h"
#include "insets/inset.h"

#include <algorithm>

using lyx::pit_type;

using std::advance;
using std::endl;


namespace {

/// The flag used by finishUndo().
bool undo_finished;


std::ostream & operator<<(std::ostream & os, Undo const & undo)
{
	return os << " from: " << undo.from << " end: " << undo.end
		<< " cell:\n" << undo.cell
		<< " cursor:\n" << undo.cursor;
}


void doRecordUndo(Undo::undo_kind kind,
	DocIterator const & cell,
	pit_type first_pit, pit_type last_pit,
	DocIterator const & cur,
	BufferParams const & bparams,
	bool isFullBuffer,
	limited_stack<Undo> & stack)
{
	if (first_pit > last_pit)
		std::swap(first_pit, last_pit);
	// create the position information of the Undo entry
	Undo undo;
	undo.kind = kind;
	undo.cell = cell;
	undo.cursor = cur;
	undo.bparams = bparams ;
	undo.isFullBuffer = isFullBuffer;
	//lyxerr << "recordUndo: cur: " << cur << endl;
	//lyxerr << "recordUndo: pos: " << cur.pos() << endl;
	//lyxerr << "recordUndo: cell: " << cell << endl;
	undo.from = first_pit;
	undo.end = cell.lastpit() - last_pit;

	// Undo::ATOMIC are always recorded (no overlapping there).
	// As nobody wants all removed character appear one by one when undoing,
	// we want combine 'similar' non-ATOMIC undo recordings to one.
	if (!undo_finished
	    && kind != Undo::ATOMIC
	    && !stack.empty()
	    && stack.top().cell == undo.cell
		  && stack.top().kind == undo.kind
		  && stack.top().from == undo.from
		  && stack.top().end == undo.end)
		return;

	// fill in the real data to be saved
	if (cell.inMathed()) {
		// simply use the whole cell
		undo.array = asString(cell.cell());
	} else {
		// some more effort needed here as 'the whole cell' of the
		// main LyXText _is_ the whole document.
		// record the relevant paragraphs
		LyXText const * text = cell.text();
		BOOST_ASSERT(text);
		ParagraphList const & plist = text->paragraphs();
		ParagraphList::const_iterator first = plist.begin();
		advance(first, first_pit);
		ParagraphList::const_iterator last = plist.begin();
		advance(last, last_pit + 1);
		undo.pars = ParagraphList(first, last);
	}

	// push the undo entry to undo stack
	//lyxerr << "undo record: " << stack.top() << std::endl;
	stack.push(undo);

	// next time we'll try again to combine entries if possible
	undo_finished = false;
}


void recordUndo(Undo::undo_kind kind,
	LCursor & cur, pit_type first_pit, pit_type last_pit,
	limited_stack<Undo> & stack)
{
	BOOST_ASSERT(first_pit <= cur.lastpit());
	BOOST_ASSERT(last_pit <= cur.lastpit());

	doRecordUndo(kind, cur, first_pit, last_pit, cur,
		cur.bv().buffer()->params(), false, stack);
}



// Returns false if no undo possible.
bool textUndoOrRedo(BufferView & bv,
	limited_stack<Undo> & stack, limited_stack<Undo> & otherstack)
{
	finishUndo();

	if (stack.empty()) {
		// Nothing to do.
		return false;
	}

	// Adjust undo stack and get hold of current undo data.
	Undo undo = stack.top();
	stack.pop();

	// We will store in otherstack the part of the document under 'undo'
	Buffer * buf = bv.buffer();
	DocIterator cell_dit = undo.cell.asDocIterator(&buf->inset());

	doRecordUndo(Undo::ATOMIC, cell_dit,
		   undo.from, cell_dit.lastpit() - undo.end, bv.cursor(),
			 undo.bparams, undo.isFullBuffer,
		   otherstack);

	// This does the actual undo/redo.
	//lyxerr << "undo, performing: " << undo << std::endl;
	DocIterator dit = undo.cell.asDocIterator(&buf->inset());
	if (undo.isFullBuffer) {
		// This is a full document
		otherstack.top().bparams = buf->params();
		buf->params() = undo.bparams;
		buf->paragraphs() = undo.pars;
	} else if (dit.inMathed()) {
		// We stored the full cell here as there is not much to be
		// gained by storing just 'a few' paragraphs (most if not
		// all math inset cells have just one paragraph!)
		asArray(undo.array, dit.cell());
	} else {
		// Some finer machinery is needed here.
		LyXText * text = dit.text();
		BOOST_ASSERT(text);
		ParagraphList & plist = text->paragraphs();

		// remove new stuff between first and last
		ParagraphList::iterator first = plist.begin();
		advance(first, undo.from);
		ParagraphList::iterator last = plist.begin();
		advance(last, plist.size() - undo.end);
		plist.erase(first, last);

		// re-insert old stuff instead
		first = plist.begin();
		advance(first, undo.from);

		// this ugly stuff is needed until we get rid of the
		// inset_owner backpointer
		ParagraphList::const_iterator pit = undo.pars.begin();
		ParagraphList::const_iterator end = undo.pars.end();
		for (; pit != end; ++pit)
			const_cast<Paragraph &>(*pit).setInsetOwner(&dit.inset());
		plist.insert(first, undo.pars.begin(), undo.pars.end());
	}

	// Set cursor
	LCursor & cur = bv.cursor();
	cur.setCursor(undo.cursor.asDocIterator(&buf->inset()));
	cur.selection() = false;
	cur.resetAnchor();
	finishUndo();

	return true;
}

} // namespace anon


void finishUndo()
{
	// Make sure the next operation will be stored.
	undo_finished = true;
}


bool textUndo(BufferView & bv)
{
	return textUndoOrRedo(bv, bv.buffer()->undostack(),
			      bv.buffer()->redostack());
}


bool textRedo(BufferView & bv)
{
	return textUndoOrRedo(bv, bv.buffer()->redostack(),
			      bv.buffer()->undostack());
}


void recordUndo(Undo::undo_kind kind,
	LCursor & cur, pit_type first, pit_type last)
{
	Buffer * buf = cur.bv().buffer();
	recordUndo(kind, cur, first, last, buf->undostack());
	buf->redostack().clear();
	//lyxerr << "undostack:\n";
	//for (size_t i = 0, n = buf->undostack().size(); i != n && i < 6; ++i)
	//	lyxerr << "  " << i << ": " << buf->undostack()[i] << std::endl;
}


void recordUndo(LCursor & cur, Undo::undo_kind kind)
{
	recordUndo(kind, cur, cur.pit(), cur.pit());
}


void recordUndoInset(LCursor & cur, Undo::undo_kind kind)
{
	LCursor c = cur;
	c.pop();
	recordUndo(c, kind);
}


void recordUndoSelection(LCursor & cur, Undo::undo_kind kind)
{
	recordUndo(kind, cur, cur.selBegin().pit(), cur.selEnd().pit());
}


void recordUndo(LCursor & cur, Undo::undo_kind kind, pit_type from)
{
	recordUndo(kind, cur, cur.pit(), from);
}


void recordUndo(LCursor & cur, Undo::undo_kind kind,
	pit_type from, pit_type to)
{
	recordUndo(kind, cur, from, to);
}


void recordUndoFullDocument(BufferView * bv)
{
	Buffer * buf = bv->buffer();
	doRecordUndo(
		Undo::ATOMIC,
		doc_iterator_begin(buf->inset()),
		0, buf->paragraphs().size() - 1,
		bv->cursor(),
		buf->params(),
		true,
		buf->undostack()
	);
	undo_finished = false;
}
