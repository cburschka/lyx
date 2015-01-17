// -*- C++ -*-
/**
 * \file Undo.h
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

#ifndef UNDO_H
#define UNDO_H

#include "support/types.h"

namespace lyx {

class Buffer;
class BufferParams;
class CursorData;
class Inset;
class MathData;
class ParagraphList;

/// This is used to combine consecutive undo recordings of the same kind.
enum UndoKind {
	/**
	* Insert something - these will combine to one big chunk
	* when many inserts come after each other.
	*/
	INSERT_UNDO,
	/**
	* Delete something - these will combine to one big chunk
	* when many deletes come after each other.
	*/
	DELETE_UNDO,
	/// Atomic - each of these will have its own entry in the stack
	ATOMIC_UNDO
};


class Undo
{
public:

	Undo(Buffer &);

	~Undo();

	/// Clear out all undo/redo contents.
	void clear();

	/// this will undo the last action - returns false if no undo possible
	bool textUndo(CursorData &);

	/// this will redo the last undo - returns false if no redo possible
	bool textRedo(CursorData &);

	/// End a sequence of INSERT_UNDO or DELETE_UNDO type of undo
	/// operations (grouping of consecutive characters insertion/deletion).
	void finishUndo();

	///
	bool hasUndoStack() const;
	///
	bool hasRedoStack() const;

	/// Mark all the elements of the undo and redo stacks as dirty
	void markDirty();

	/// open a new group of undo operations.
	/**
	 *  Groups can be nested. Such a nested group e.g. { {} {} } is undone in
	 *  a single step. This means you can add a group whenever you are not sure.
	 */
	void beginUndoGroup();

	/// end the current undo group.
	void endUndoGroup();

	/// end the current undo group and set UndoElement::cur_after if necessary.
	void endUndoGroup(CursorData const &);

	/// The general case: record undo information for an arbitrary range.
	/**
	 * Record undo information - call with the current cursor and
	 * the 'other end' of the range of changed paragraphs. So we
	 * give an inclusive range. This is called before you make the
	 * changes to the paragraph, and it will record the original
	 * information of the paragraphs in the undo stack.
	 */
	void recordUndo(CursorData const & cur, UndoKind kind,
		pit_type from, pit_type to);

	/// Convenience: record undo information for the range between
	/// 'from' and cursor.
	void recordUndo(CursorData const & cur, UndoKind kind, pit_type from);

	/// Convenience: record undo information for the single
	/// paragraph or cell containing the cursor.
	void recordUndo(CursorData const & cur, UndoKind kind = ATOMIC_UNDO);

	/// Convenience: record undo information for the inset
	/// containing the cursor.
	void recordUndoInset(CursorData const & cur,
			     UndoKind kind = ATOMIC_UNDO,
			     Inset const * inset = 0);

	/// Convenience: record undo for buffer parameters
	void recordUndoBufferParams(CursorData const & cur);

	/// Convenience: prepare undo for the whole buffer
	void recordUndoFullBuffer(CursorData const & cur);

private:
	struct Private;
	Private * const d;
};



} // namespace lyx

#endif // UNDO_H
