// -*- C++ -*-
/**
 * \file undo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef UNDO_H
#define UNDO_H

#include "ParagraphList_fwd.h"

/**
 * These are the elements put on the undo stack. Each object
 * contains complete paragraphs and sufficient information
 * to restore the state. The work is done in undo_funcs.C
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
	Undo(undo_kind kind, int inset_id,
	     int first, int last,
	     int cursor, int cursor_pos,
	     ParagraphList const & par_arg);

	/// Which kind of operation are we recording for?
	undo_kind kind;

	/**
	 * ID of hosting inset if the cursor is in one.
	 * if -1, then the cursor is not in an inset.
	 * if >= 0, then the cursor is in inset with given id.
	 */
	int inset_id;

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


#endif
