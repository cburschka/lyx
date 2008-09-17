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
class DocIterator;
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

	/// this will undo the last action - returns false if no undo possible
	bool textUndo(DocIterator &);

	/// this will redo the last undo - returns false if no redo possible
	bool textRedo(DocIterator &);

	/// End a sequence of INSERT_UNDO or DELETE_UNDO type of undo
	/// operations.
	void finishUndo();

	///
	bool hasUndoStack() const;
	///
	bool hasRedoStack() const;

	/// open a new group of undo operations. Groups can be nested.
	void beginUndoGroup();

	/// end the current undo group.
	void endUndoGroup();

	/// The general case: record undo information for an arbitrary range.
	/**
	 * Record undo information - call with the current cursor and
	 * the 'other end' of the range of changed paragraphs. So we
	 * give an inclusive range. This is called before you make the
	 * changes to the paragraph, and it will record the original
	 * information of the paragraphs in the undo stack.
	 */
	void recordUndo(DocIterator const & cur, UndoKind kind,
		pit_type from, pit_type to);

	/// Convenience: record undo information for the range between
	/// 'from' and cursor.
	void recordUndo(DocIterator const & cur, UndoKind kind, pit_type from);

	/// Convenience: record undo information for the single
	/// paragraph or cell containing the cursor.
	void recordUndo(DocIterator const & cur, UndoKind kind = ATOMIC_UNDO);

	/// Convenience: record undo information for the inset
	/// containing the cursor.
	void recordUndoInset(DocIterator const & cur, 
			     UndoKind kind = ATOMIC_UNDO);

	/// Convenience: prepare undo for the whole buffer
	void recordUndoFullDocument(DocIterator const & cur);

private:
	struct Private;
	Private * const d;
};



} // namespace lyx

#endif // UNDO_H
