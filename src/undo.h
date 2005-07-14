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
#include "bufferparams.h"

#include "support/types.h"

#include <string>

class BufferParams;
class BufferView;
class LCursor;


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

class Undo {
public:
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

	/// Which kind of operation are we recording for?
	undo_kind kind;
	/// the position of the cursor
	StableDocIterator cursor;
	/// the position of the cell described
	StableDocIterator cell;
	/// counted from begin of cell
	lyx::pit_type from;
	/// complement to end of this cell
	lyx::pit_type end;
	/// the contents of the saved Paragraphs (for texted)
	ParagraphList pars;
	/// the stringified contents of the saved MathArray (for mathed)
	std::string array;
	/// Only used in case of full backups
	BufferParams bparams;
	/// Only used in case of full backups
	bool isFullBuffer;	
};


/// this will undo the last action - returns false if no undo possible
bool textUndo(BufferView & bv);

/// this will redo the last undo - returns false if no redo possible
bool textRedo(BufferView & bv);

/// makes sure the next operation will be stored
void finishUndo();


/**
 * Record undo information - call with the current cursor and the 'other
 * end' of the range of changed  paragraphs.  So we give an inclusive range.
 * This is called before you make the changes to the paragraph, and it
 * will record the original information of the paragraphs in the undo stack.
 */

/// The general case: prepare undo for an arbitrary range.
void recordUndo(LCursor & cur, Undo::undo_kind kind,
	lyx::pit_type from, lyx::pit_type to);

/// Convenience: prepare undo for the range between 'from' and cursor.
void recordUndo(LCursor & cur, Undo::undo_kind kind, lyx::pit_type from);

/// Convenience: prepare undo for the single paragraph or cell
/// containing the cursor
void recordUndo(LCursor & cur, Undo::undo_kind kind = Undo::ATOMIC);
/// Convenience: prepare undo for the inset containing the cursor
void recordUndoInset(LCursor & cur, Undo::undo_kind kind = Undo::ATOMIC);
/// Convenience: prepare undo for the selected paragraphs
void recordUndoSelection(LCursor & cur, Undo::undo_kind kind = Undo::ATOMIC);

/// Convenience: prepare undo for the whole buffer
void recordUndoFullDocument(BufferView * bv);

#endif // UNDO_FUNCS_H
