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

#include <algorithm>

using std::advance;

using lyx::par_type;

using std::endl;


namespace {

/// The flag used by finishUndo().
bool undo_finished;


std::ostream & operator<<(std::ostream & os, Undo const & undo)
{
	return os << " from: " << undo.from
		<< " end: " << undo.end
		<< " cursor:\n" << undo.cursor;
}


void recordUndo(Undo::undo_kind kind,
	LCursor & cur, par_type first_par, par_type last_par,
	limited_stack<Undo> & stack)
{
	BOOST_ASSERT(first_par <= cur.lastpar());
	BOOST_ASSERT(last_par <= cur.lastpar());

	if (first_par > last_par)
		std::swap(first_par, last_par);

	// create the position information of the Undo entry
	Undo undo;
	undo.kind = kind;
	undo.cursor = StableDocIterator(cur);
	undo.from = first_par;
	undo.end = cur.lastpar() - last_par;

	// Undo::ATOMIC are always recorded (no overlapping there).
	// As nobody wants all removed character appear one by one when undoing,
	// we want combine 'similar' non-ATOMIC undo recordings to one.
	if (!undo_finished
	    && kind != Undo::ATOMIC
	    && !stack.empty()
	    && stack.top().cursor.size() == undo.cursor.size()
		  && stack.top().kind == undo.kind
		  && stack.top().from == undo.from
		  && stack.top().end == undo.end)
		return;

	// fill in the real data to be saved
	if (cur.inMathed()) {
		// simply use the whole cell
		undo.array = asString(cur.cell());
	} else {
		// some more effort needed here as 'the whole cell' of the
		// main LyXText _is_ the whole document.
		// record the relevant paragraphs
		LyXText * text = cur.text();
		BOOST_ASSERT(text);
		ParagraphList & plist = text->paragraphs();
		ParagraphList::iterator first = plist.begin();
		advance(first, first_par);
		ParagraphList::iterator last = plist.begin();
		advance(last, last_par + 1);
		undo.pars = ParagraphList(first, last);
	}

	// push the undo entry to undo stack 
	//lyxerr << "undo record: " << stack.top() << std::endl;
	stack.push(undo);

	// next time we'll try again to combine entries if possible
	undo_finished = false;
}


void performUndoOrRedo(BufferView & bv, Undo const & undo)
{
	LCursor & cur = bv.cursor();
	lyxerr << "undo, performing: " << undo << std::endl;
	cur.setCursor(undo.cursor.asDocIterator(&bv.buffer()->inset()), false);

	if (cur.inMathed()) {
		// We stored the full cell here as there is not much to be
		// gained by storing just 'a few' paragraphs (most if not
		// all math inset cells have just one paragraph!)
		asArray(undo.array, cur.cell());
	} else {
		// Some finer machinery is needed here.
		LyXText * text = cur.text();
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
		plist.insert(first, undo.pars.begin(), undo.pars.end());
	}

	cur.resetAnchor();
	finishUndo();
}


// returns false if no undo possible
bool textUndoOrRedo(BufferView & bv,
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

	// this implements redo
	otherstack.push(undo);
	DocIterator dit =
		undo.cursor.asDocIterator(&bv.buffer()->inset());
	if (dit.inMathed()) {
		// Easy way out: store a full cell.
		otherstack.top().array = asString(dit.cell());
	} else {
		// As cells might be too large in texted, store just a part
		// of the paragraph list.
		otherstack.top().pars.clear();
		LyXText * text = dit.text();
		BOOST_ASSERT(text);
		ParagraphList & plist = text->paragraphs();
		if (undo.from + undo.end <= int(plist.size())) {
			ParagraphList::iterator first = plist.begin();
			advance(first, undo.from);
			ParagraphList::iterator last = plist.begin();
			advance(last, plist.size() - undo.end);
			otherstack.top().pars.insert(otherstack.top().pars.begin(), first, last);
		}
	}
	otherstack.top().cursor = bv.cursor();
	//lyxerr << " undo other: " << otherstack.top() << std::endl;

	performUndoOrRedo(bv, undo);
	return true;
}

} // namespace anon


void finishUndo()
{
	// makes sure the next operation will be stored
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
	LCursor & cur, par_type first, par_type last)
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
	recordUndo(kind, cur, cur.par(), cur.par());
}


void recordUndoSelection(LCursor & cur, Undo::undo_kind kind)
{
	recordUndo(kind, cur, cur.selBegin().par(), cur.selEnd().par());
}


void recordUndo(LCursor & cur, Undo::undo_kind kind, par_type from)
{
	recordUndo(kind, cur, cur.par(), from);
}


void recordUndo(LCursor & cur, Undo::undo_kind kind,
	par_type from, par_type to)
{
	recordUndo(kind, cur, from, to);
}


void recordUndoFullDocument(LCursor &)
{
	//recordUndo(Undo::ATOMIC,
	//	cur, 0, cur.bv().text()->paragraphs().size() - 1);
}
