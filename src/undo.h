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

class LyXText;
class BufferView;

/**
 * These are the elements put on the undo stack. Each object
 * contains complete paragraphs and sufficient information
 * to restore the state.
 */
class Undo {
public:
	/**
	 * The undo kinds are used to combine consecutive undo recordings
	 * of the same kind.
	 */
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
	///
	Undo(undo_kind kind, int text,
	     int first, int last,
	     int cursor, int cursor_pos,
	     ParagraphList const & par_arg);

	/// Which kind of operation are we recording for?
	undo_kind kind;

	/// hosting LyXText counted from buffer begin
	int text;

	/// Offset to the first paragraph in the main document paragraph list
	int first_par_offset;

	/// Offset to the last paragraph from the end of the main par. list
	int last_par_offset;

	/**
	 * Offset from the start of the main document paragraph list,
	 * except if inside an inset, in which case it's the offset
	 * inside the hosting inset.
	 */
	int cursor_par_offset;

	/// The position of the cursor in the hosting paragraph
	int cursor_pos;

	/// The contents of the paragraphs saved
	ParagraphList pars;
};


/// This will undo the last action - returns false if no undo possible
bool textUndo(BufferView *);

/// This will redo the last undo - returns false if no redo possible
bool textRedo(BufferView *);

/// Makes sure the next operation will be stored
void finishUndo();

/**
 * Whilst undo is frozen, all actions do not get added
 * to the undo stack
 */
void freezeUndo();

/// Track undos again
void unFreezeUndo();

/**
 * Record undo information - call with the first paragraph that will be changed
 * and the last paragraph that will be changed. So we give an inclusive
 * range.
 * This is called before you make the changes to the paragraph, and it
 * will record the original information of the paragraphs in the undo stack.
 */
void recordUndo(Undo::undo_kind kind,
	LyXText const * text, lyx::paroffset_type first, lyx::paroffset_type last);

/// Convienience: Prepare undo when change in a single paragraph.
void recordUndo(Undo::undo_kind kind,
	LyXText const * text, lyx::paroffset_type par);

/// Convienience: Prepare undo for the paragraph that contains the cursor
void recordUndo(BufferView *, Undo::undo_kind kind);

/// Are we avoiding tracking undos currently ?
extern bool undo_frozen;

#endif // UNDO_FUNCS_H
