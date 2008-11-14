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
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Undo.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "buffer_funcs.h"
#include "DocIterator.h"
#include "Paragraph.h"
#include "ParagraphList.h"
#include "Text.h"

#include "mathed/MathSupport.h"
#include "mathed/MathData.h"

#include "insets/Inset.h"

#include "support/lassert.h"
#include "support/debug.h"

#include <algorithm>
#include <deque>

using namespace std;
using namespace lyx::support;


namespace lyx {

/**
These are the elements put on the undo stack. Each object contains complete
paragraphs from some cell and sufficient information to restore the cursor
state.

The cell is given by a DocIterator pointing to this cell, the 'interesting'
range of paragraphs by counting them from begin and end of cell,
respectively.

The cursor is also given as DocIterator and should point to some place in
the stored paragraph range.  In case of math, we simply store the whole
cell, as there usually is just a simple paragraph in a cell.

The idea is to store the contents of 'interesting' paragraphs in some
structure ('Undo') _before_ it is changed in some edit operation.
Obviously, the stored ranged should be as small as possible. However, it
there is a lower limit: The StableDocIterator pointing stored in the undo
class must be valid after the changes, too, as it will used as a pointer
where to insert the stored bits when performining undo.
*/


struct UndoElement
{
	///
	UndoElement(UndoKind kin, StableDocIterator const & cur, 
	            StableDocIterator const & cel,
	            pit_type fro, pit_type en, ParagraphList * pl, 
	            MathData * ar, BufferParams const & bp, 
	            bool ifb, size_t gid) :
	        kind(kin), cursor(cur), cell(cel), from(fro), end(en),
	        pars(pl), array(ar), bparams(0), isFullBuffer(ifb), group_id(gid)
	{
		if (isFullBuffer)
			bparams = new BufferParams(bp);
	}
	///
	UndoElement(UndoElement const & ue)
	{
		kind = ue.kind;
		cursor = ue.cursor;
		cell = ue.cell;
		from = ue.from;
		end = ue.end;
		pars = ue.pars;
		array = ue.array;
		bparams = ue.isFullBuffer
			? new BufferParams(*ue.bparams) : ue.bparams;
		isFullBuffer = ue.isFullBuffer;
		group_id = ue.group_id;
	}
	///
	~UndoElement()
	{
		if (isFullBuffer)
			delete bparams;
	}
	/// Which kind of operation are we recording for?
	UndoKind kind;
	/// the position of the cursor
	StableDocIterator cursor;
	/// the position of the cell described
	StableDocIterator cell;
	/// counted from begin of cell
	pit_type from;
	/// complement to end of this cell
	pit_type end;
	/// the contents of the saved Paragraphs (for texted)
	ParagraphList * pars;
	/// the contents of the saved MathData (for mathed)
	MathData * array;
	/// Only used in case of full backups
	BufferParams const * bparams;
	/// Only used in case of full backups
	bool isFullBuffer;
	/// the element's group id
	size_t group_id;
private:
	/// Protect construction
	UndoElement();	
};


class UndoElementStack 
{
public:
	/// limit is the maximum size of the stack
	UndoElementStack(size_t limit = 100) { limit_ = limit; }
	/// limit is the maximum size of the stack
	~UndoElementStack() { clear(); }

	/// Return the top element.
	UndoElement & top() { return c_.front(); }

	/// Pop and throw away the top element.
	void pop() { c_.pop_front(); }

	/// Return true if the stack is empty.
	bool empty() const { return c_.empty(); }

	/// Clear all elements, deleting them.
	void clear() {
		for (size_t i = 0; i != c_.size(); ++i) {
			delete c_[i].array;
			delete c_[i].pars;
		}
		c_.clear();
	}

	/// Push an item on to the stack, deleting the bottom group on
	/// overflow.
	void push(UndoElement const & v) {
		c_.push_front(v);
		if (c_.size() > limit_) {
			// remove a whole group at once.
			const size_t gid = c_.back().group_id;
			while (!c_.empty() && c_.back().group_id == gid)
				c_.pop_back();
		}
	}

private:
	/// Internal contents.
	std::deque<UndoElement> c_;
	/// The maximum number elements stored.
	size_t limit_;
};


struct Undo::Private
{
	Private(Buffer & buffer) : buffer_(buffer), undo_finished_(true), 
				   group_id(0), group_level(0) {}
	
	// Do one undo/redo step
	void doTextUndoOrRedo(DocIterator & cur, UndoElementStack & stack, UndoElementStack & otherStack);
	// Apply one undo/redo group. Returns false if no undo possible.
	bool textUndoOrRedo(DocIterator & cur, bool isUndoOperation);

	///
	void doRecordUndo(UndoKind kind,
		DocIterator const & cell,
		pit_type first_pit,
		pit_type last_pit,
		DocIterator const & cur,
		bool isFullBuffer,
		UndoElementStack & stack);
	///
	void recordUndo(UndoKind kind,
		DocIterator const & cur,
		pit_type first_pit,
		pit_type last_pit);

	///
	Buffer & buffer_;
	/// Undo stack.
	UndoElementStack undostack_;
	/// Redo stack.
	UndoElementStack redostack_;

	/// The flag used by Undo::finishUndo().
	bool undo_finished_;

	/// Current group Id.
	size_t group_id;
	/// Current group nesting nevel.
	size_t group_level;
};


/////////////////////////////////////////////////////////////////////
//
// Undo
//
/////////////////////////////////////////////////////////////////////


Undo::Undo(Buffer & buffer)
	: d(new Undo::Private(buffer))
{}


Undo::~Undo()
{
	delete d;
}


bool Undo::hasUndoStack() const
{
	return !d->undostack_.empty();
}


bool Undo::hasRedoStack() const
{
	return !d->redostack_.empty();
}


static bool samePar(StableDocIterator const & i1, StableDocIterator const & i2)
{
	StableDocIterator tmpi2 = i2;
	tmpi2.pos() = i1.pos();
	return i1 == tmpi2;
}


/////////////////////////////////////////////////////////////////////
//
// Undo::Private
//
///////////////////////////////////////////////////////////////////////

void Undo::Private::doRecordUndo(UndoKind kind,
	DocIterator const & cell,
	pit_type first_pit, pit_type last_pit,
	DocIterator const & cur,
	bool isFullBuffer,
	UndoElementStack & stack)
{
	if (!group_level) {
		LYXERR0("There is no group open (creating one)");
		++group_id;
	}

	if (first_pit > last_pit)
		swap(first_pit, last_pit);

	// Undo::ATOMIC are always recorded (no overlapping there).
	// As nobody wants all removed character appear one by one when undoing,
	// we want combine 'similar' non-ATOMIC undo recordings to one.
	pit_type from = first_pit;
	pit_type end = cell.lastpit() - last_pit;
	if (!undo_finished_
	    && kind != ATOMIC_UNDO
	    && !stack.empty()
	    && samePar(stack.top().cell, cell)
	    && stack.top().kind == kind
	    && stack.top().from == from
	    && stack.top().end == end)
		return;

	if (isFullBuffer)
		LYXERR(Debug::UNDO, "Create full buffer undo element of group " << group_id);
	else
		LYXERR(Debug::UNDO, "Create undo element of group " << group_id);
	// create the position information of the Undo entry
	UndoElement undo(kind, cur, cell, from, end, 0, 0, 
	                 buffer_.params(), isFullBuffer, group_id);

	// fill in the real data to be saved
	if (cell.inMathed()) {
		// simply use the whole cell
		undo.array = new MathData(cell.cell());
	} else {
		// some more effort needed here as 'the whole cell' of the
		// main Text _is_ the whole document.
		// record the relevant paragraphs
		Text const * text = cell.text();
		LASSERT(text, /**/);
		ParagraphList const & plist = text->paragraphs();
		ParagraphList::const_iterator first = plist.begin();
		advance(first, first_pit);
		ParagraphList::const_iterator last = plist.begin();
		advance(last, last_pit + 1);
		undo.pars = new ParagraphList(first, last);
	}

	// push the undo entry to undo stack
	stack.push(undo);
	//lyxerr << "undo record: " << stack.top() << endl;

	// next time we'll try again to combine entries if possible
	undo_finished_ = false;
}


void Undo::Private::recordUndo(UndoKind kind, DocIterator const & cur,
	pit_type first_pit, pit_type last_pit)
{
	LASSERT(first_pit <= cur.lastpit(), /**/);
	LASSERT(last_pit <= cur.lastpit(), /**/);

	doRecordUndo(kind, cur, first_pit, last_pit, cur,
		false, undostack_);

	undo_finished_ = false;
	redostack_.clear();
	//lyxerr << "undostack:\n";
	//for (size_t i = 0, n = buf.undostack().size(); i != n && i < 6; ++i)
	//	lyxerr << "  " << i << ": " << buf.undostack()[i] << endl;
}


void Undo::Private::doTextUndoOrRedo(DocIterator & cur, UndoElementStack & stack, UndoElementStack & otherstack)
{
	// Adjust undo stack and get hold of current undo data.
	UndoElement & undo = stack.top();
	LYXERR(Debug::UNDO, "Undo element of group " << undo.group_id);
	// We'll pop the stack only when we're done with this element. So do NOT
	// try to return early.

	// We will store in otherstack the part of the document under 'undo'
	DocIterator cell_dit = undo.cell.asDocIterator(&buffer_.inset());

	doRecordUndo(ATOMIC_UNDO, cell_dit,
		undo.from, cell_dit.lastpit() - undo.end, cur,
		undo.isFullBuffer, otherstack);

	// This does the actual undo/redo.
	//LYXERR0("undo, performing: " << undo);
	DocIterator dit = undo.cell.asDocIterator(&buffer_.inset());
	if (undo.isFullBuffer) {
		LASSERT(undo.pars, /**/);
		// This is a full document
		delete otherstack.top().bparams;
		otherstack.top().bparams = new BufferParams(buffer_.params());
		buffer_.params() = *undo.bparams;
		swap(buffer_.paragraphs(), *undo.pars);
		delete undo.pars;
		undo.pars = 0;
	} else if (dit.inMathed()) {
		// We stored the full cell here as there is not much to be
		// gained by storing just 'a few' paragraphs (most if not
		// all math inset cells have just one paragraph!)
		//LYXERR0("undo.array: " << *undo.array);
		LASSERT(undo.array, /**/);
		dit.cell().swap(*undo.array);
		delete undo.array;
		undo.array = 0;
	} else {
		// Some finer machinery is needed here.
		Text * text = dit.text();
		LASSERT(text, /**/);
		LASSERT(undo.pars, /**/);
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
	}
	LASSERT(undo.pars == 0, /**/);
	LASSERT(undo.array == 0, /**/);

	cur = undo.cursor.asDocIterator(&buffer_.inset());
	// Now that we're done with undo, we pop it off the stack.
	stack.pop();
}


bool Undo::Private::textUndoOrRedo(DocIterator & cur, bool isUndoOperation)
{
	undo_finished_ = true;

	UndoElementStack & stack = isUndoOperation ?  undostack_ : redostack_;

	if (stack.empty())
		// Nothing to do.
		return false;

	UndoElementStack & otherstack = isUndoOperation ? redostack_ : undostack_;

	const size_t gid = stack.top().group_id;
	while (!stack.empty() && stack.top().group_id == gid)
		doTextUndoOrRedo(cur, stack, otherstack);

	// Addapt the new material to current buffer.
	updateLabels(buffer_);
	return true;
}


void Undo::finishUndo()
{
	// Make sure the next operation will be stored.
	d->undo_finished_ = true;
}


bool Undo::textUndo(DocIterator & cur)
{
	return d->textUndoOrRedo(cur, true);
}


bool Undo::textRedo(DocIterator & cur)
{
	return d->textUndoOrRedo(cur, false);
}


void Undo::beginUndoGroup()
{
	if (d->group_level == 0) {
		// create a new group
		++d->group_id;
		LYXERR(Debug::UNDO, "+++++++Creating new group " << d->group_id);
	}
	++d->group_level;
}


void Undo::endUndoGroup()
{
	if (d->group_level == 0)
		LYXERR0("There is no undo group to end here");
	--d->group_level;
	if (d->group_level == 0) {
		// real end of the group
		LYXERR(Debug::UNDO, "-------End of group " << d->group_id);
	}
}



void Undo::recordUndo(DocIterator const & cur, UndoKind kind)
{
	d->recordUndo(kind, cur, cur.pit(), cur.pit());
}


void Undo::recordUndoInset(DocIterator const & cur, UndoKind kind)
{
	DocIterator c = cur;
	c.pop_back();
	d->doRecordUndo(kind, c, c.pit(), c.pit(), cur, false, d->undostack_);
}


void Undo::recordUndo(DocIterator const & cur, UndoKind kind, pit_type from)
{
	d->recordUndo(kind, cur, cur.pit(), from);
}


void Undo::recordUndo(DocIterator const & cur, UndoKind kind,
	pit_type from, pit_type to)
{
	d->recordUndo(kind, cur, from, to);
}


void Undo::recordUndoFullDocument(DocIterator const & cur)
{
	// This one may happen outside of the main undo group, so we
	// put it in its own subgroup to avoid complaints.
	beginUndoGroup();
	d->doRecordUndo(
		ATOMIC_UNDO,
		doc_iterator_begin(d->buffer_.inset()),
		0, d->buffer_.paragraphs().size() - 1,
		cur,
		true,
		d->undostack_
	);
	endUndoGroup();
}


} // namespace lyx
