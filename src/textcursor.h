// -*- C++ -*-
/**
 * \file textcursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXTCURSOR_H
#define TEXTCURSOR_H

#include "lyxcursor.h"

// Do not even think of forward declaring LyXText/BufferView etc here!
// If you need Paragraph proper, go to text_func.h

/** The cursor.
	Later this variable has to be removed. There should be no internal
	cursor in a text (and thus not in a buffer). By keeping this it is
	(I think) impossible to have several views with the same buffer, but
	the cursor placed at different places.
	[later]
	Since the LyXText now has been moved from Buffer to BufferView
	it should not be absolutely needed to move the cursor...
	[even later]
	Nevertheless, it should still be moved, in order to keep classes
	and interdependencies small.
	*/

// The structure that keeps track of the selections set.
struct Selection {
	Selection()
		: set_(false), mark_(false)
		{}
	bool set() const {
		return set_;
	}
	void set(bool s) {
		set_ = s;
	}
	bool mark() const {
		return mark_;
	}
	void mark(bool m) {
		mark_ = m;
	}
private:
	bool set_; // former selection
	bool mark_; // former mark_set

};

struct TextCursor {
	///
	void setSelection();
	///
	void clearSelection();

	// actual cursor position
	LyXCursor cursor_;
	// the other end of the selection
	LyXCursor anchor_;

	Selection selection;

	LyXCursor const & selStart() const;
	LyXCursor const & selEnd() const;
	LyXCursor & selStart();
	LyXCursor & selEnd();
};

#endif
