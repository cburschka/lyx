// -*- C++ -*-
/**
 * \file undo.h
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

#ifndef UNDO_H
#define UNDO_H

#include "dociterator.h"
#include "ParagraphList_fwd.h"

#include "support/types.h"

#include <string>

class LCursor;
class BufferView;


/**
 * These are the elements put on the undo stack. Each object
 * contains complete paragraphs and sufficient information
 * to restore the state.
 */
struct Undo {
	/// This is used to combine consecutive undo recordings of the same kind.
	enum undo_kind {
		/**
		 * Insert something - these will combine to one big chunk
		 * when many inserts come after each other.
		 */
		INSERT,
		/**
		 * Delete something - these will combine to one big chunk
		 * when many deletes come after each other.
		 */
		DELETE,
		/// Atomic - each of these will have its own entry in the stack
		ATOMIC
	};

	/// which kind of operation are we recording for?
	undo_kind kind;
	/// the position of the cursor
	StableDocIterator cursor;
	/// counted from begin of buffer
	lyx::par_type from;
	/// complement to end of this cell
	lyx::par_type end;
	/// the contents of the saved paragraphs (for texted)
	ParagraphList pars;
	/// the contents of the saved matharray (for mathed)
	std::string array;
};


/// this will undo the last action - returns false if no undo possible
bool textUndo(BufferView &);

/// this will redo the last undo - returns false if no redo possible
bool textRedo(BufferView &);

/// makes sure the next operation will be stored
void finishUndo();


/**
 * Record undo information - call with the current cursor and the 'other
 * end' of the range of changed  paragraphs.  So we give an inclusive range.
 * This is called before you make the changes to the paragraph, and it
 * will record the original information of the paragraphs in the undo stack.
 */

/// the common case: prepare undo for an arbitrary range
void recordUndo(LCursor & cur, Undo::undo_kind kind,
	lyx::par_type from, lyx::par_type to);

/// convienience: prepare undo for the range between 'from' and cursor.
void recordUndo(LCursor & cur, Undo::undo_kind kind, lyx::par_type from);

/// convienience: prepare undo for the single paragraph containing the cursor
void recordUndo(LCursor & cur, Undo::undo_kind kind = Undo::ATOMIC);
/// convienience: prepare undo for the selected paragraphs
void recordUndoSelection(LCursor & cur, Undo::undo_kind kind = Undo::ATOMIC);

/// convienience: prepare undo for the single paragraph containing the cursor
void recordUndoFullDocument(LCursor & cur);

#endif // UNDO_FUNCS_H
