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

#include "ParagraphList_fwd.h"
#include "support/types.h"

class LCursor;
class BufferView;


/**
 * These are the elements put on the undo stack. Each object
 * contains complete paragraphs and sufficient information
 * to restore the state.
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

	/// constructor
	Undo(undo_kind kind, int text, int index,
		int first_par, int end_par, int cursor_par, int cursor_pos);

public:
	/// which kind of operation are we recording for?
	undo_kind kind;
	/// hosting LyXText counted from buffer begin
	int text;
	/// cell in a tabular or similar
	int index;
	/// offset to the first paragraph in the paragraph list
	int first_par;
	/// offset to the last paragraph from the end of paragraph list
	int end_par;
	/// offset to the first paragraph in the paragraph list
	int cursor_par;
	/// the position of the cursor in the hosting paragraph
	int cursor_pos;
	/// the contents of the paragraphs saved
	ParagraphList pars;
};


/// this will undo the last action - returns false if no undo possible
bool textUndo(BufferView &);

/// this will redo the last undo - returns false if no redo possible
bool textRedo(BufferView &);

/// makes sure the next operation will be stored
void finishUndo();

/// whilst undo is frozen, all actions do not get added to the undo stack
void freezeUndo();

/// track undos again
void unFreezeUndo();


/**
 * Record undo information - call with the current cursor and the 'other
 * end' of the range of changed  paragraphs.  So we give an inclusive range.
 * This is called before you make the changes to the paragraph, and it
 * will record the original information of the paragraphs in the undo stack.
 */

/// the common case: prepare undo for an arbitrary range
void recordUndo(LCursor & cur, Undo::undo_kind kind,
	lyx::paroffset_type from, lyx::paroffset_type to);

/// convienience: prepare undo for the range between 'from' and cursor.
void recordUndo(LCursor & cur, Undo::undo_kind kind, lyx::paroffset_type from);

/// convienience: prepare undo for the single paragraph containing the cursor
void recordUndo(LCursor & cur, Undo::undo_kind kind);

/// convienience: prepare undo for the single paragraph containing the cursor
void recordUndoFullDocument(LCursor & cur);

/// are we avoiding tracking undos currently?
extern bool undo_frozen;

#endif // UNDO_FUNCS_H
