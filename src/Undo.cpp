/**
 * \file Undo.cpp
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

#include "Undo.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "BufferView.h"
#include "Text.h"
#include "Paragraph.h"
#include "ParagraphList.h"

#include "mathed/MathSupport.h"
#include "mathed/MathData.h"

#include "insets/Inset.h"

#include <algorithm>


namespace lyx {

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


bool samePar(StableDocIterator const & i1, StableDocIterator const & i2)
{
	StableDocIterator tmpi2 = i2;
	tmpi2.pos() = i1.pos();
	return i1 == tmpi2;
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
	undo.array = 0;
	undo.pars = 0;
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
	    && samePar(stack.top().cell, undo.cell)
	    && stack.top().kind == undo.kind
	    && stack.top().from == undo.from
	    && stack.top().end == undo.end)
		return;

	// fill in the real data to be saved
	if (cell.inMathed()) {
		// simply use the whole cell
		undo.array = new MathData(cell.cell());
	} else {
		// some more effort needed here as 'the whole cell' of the
		// main Text _is_ the whole document.
		// record the relevant paragraphs
		Text const * text = cell.text();
		BOOST_ASSERT(text);
		ParagraphList const & plist = text->paragraphs();
		ParagraphList::const_iterator first = plist.begin();
		advance(first, first_pit);
		ParagraphList::const_iterator last = plist.begin();
		advance(last, last_pit + 1);
		undo.pars = new ParagraphList(first, last);
	}

	// push the undo entry to undo stack
	stack.push(undo);
	//lyxerr << "undo record: " << stack.top() << std::endl;

	// next time we'll try again to combine entries if possible
	undo_finished = false;
}


void recordUndo(Undo::undo_kind kind,
	Cursor & cur, pit_type first_pit, pit_type last_pit,
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
	bool labelsUpdateNeeded = false;
	DocIterator dit = undo.cell.asDocIterator(&buf->inset());
	if (undo.isFullBuffer) {
		BOOST_ASSERT(undo.pars);
		// This is a full document
		otherstack.top().bparams = buf->params();
		buf->params() = undo.bparams;
		std::swap(buf->paragraphs(), *undo.pars);
		delete undo.pars;
		undo.pars = 0;
	} else if (dit.inMathed()) {
		// We stored the full cell here as there is not much to be
		// gained by storing just 'a few' paragraphs (most if not
		// all math inset cells have just one paragraph!)
		//lyxerr << "undo.array: " << *undo.array <<endl;
		BOOST_ASSERT(undo.array);
		dit.cell().swap(*undo.array);
		delete undo.array;
		undo.array = 0;
	} else {
		// Some finer machinery is needed here.
		Text * text = dit.text();
		BOOST_ASSERT(text);
		BOOST_ASSERT(undo.pars);
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
		ParagraphList::iterator pit = undo.pars->begin();
		ParagraphList::iterator const end = undo.pars->end();
		for (; pit != end; ++pit)
			pit->setInsetOwner(dit.realInset());
		plist.insert(first, undo.pars->begin(), undo.pars->end());
		delete undo.pars;
		undo.pars = 0;
		labelsUpdateNeeded = true;
	}
	BOOST_ASSERT(undo.pars == 0);
	BOOST_ASSERT(undo.array == 0);

	// Set cursor
	Cursor & cur = bv.cursor();
	cur.setCursor(undo.cursor.asDocIterator(&buf->inset()));
	cur.selection() = false;
	cur.resetAnchor();
	cur.fixIfBroken();
	
	if (labelsUpdateNeeded)
		updateLabels(*buf);
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
	Cursor & cur, pit_type first, pit_type last)
{
	Buffer * buf = cur.bv().buffer();
	recordUndo(kind, cur, first, last, buf->undostack());
	buf->redostack().clear();
	//lyxerr << "undostack:\n";
	//for (size_t i = 0, n = buf->undostack().size(); i != n && i < 6; ++i)
	//	lyxerr << "  " << i << ": " << buf->undostack()[i] << std::endl;
}


void recordUndo(Cursor & cur, Undo::undo_kind kind)
{
	recordUndo(kind, cur, cur.pit(), cur.pit());
}


void recordUndoInset(Cursor & cur, Undo::undo_kind kind)
{
	Cursor c = cur;
	c.pop();
	Buffer * buf = cur.bv().buffer();
	doRecordUndo(kind, c, c.pit(), c.pit(),	cur,
		     buf->params(), false, buf->undostack());
}


void recordUndoSelection(Cursor & cur, Undo::undo_kind kind)
{
	if (cur.inMathed()) {
		if (cap::multipleCellsSelected(cur))
			recordUndoInset(cur, kind);
		else
			recordUndo(cur, kind);
	} else
		recordUndo(kind, cur, cur.selBegin().pit(),
			cur.selEnd().pit());
}


void recordUndo(Cursor & cur, Undo::undo_kind kind, pit_type from)
{
	recordUndo(kind, cur, cur.pit(), from);
}


void recordUndo(Cursor & cur, Undo::undo_kind kind,
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


} // namespace lyx
