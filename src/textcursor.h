// -*- C++ -*-
/**
 * \file cursor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include "lyxcursor.h"

#ifndef TEXTCURSOR_H
#define TEXTCURSOR_H

// Do not even think of forward declaring LyXText/BufferView etc here!
// If you need Paragraph proper, go to text_func.h

/** The cursor.
	Later this variable has to be removed. There should be now internal
	cursor in a text (and thus not in a buffer). By keeping this it is
	(I think) impossible to have several views with the same buffer, but
	the cursor placed at different places.
	[later]
	Since the LyXText now has been moved from Buffer to BufferView
	it should not be absolutely needed to move the cursor...
	[even later]
	It should neverthe less to keep classes and intedependencies small 
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
	LyXCursor cursor; // temporary cursor to hold a cursor position
				// until setSelection is called!
	LyXCursor start;  // start of a REAL selection
	LyXCursor end;    // end of a REAL selection
private:
	bool set_; // former selection
	bool mark_; // former mark_set

};

struct TextCursor {
	/// returns true if selection was set previously
	bool setSelection();
	///
	void clearSelection();
	///
	string const selectionAsString(Buffer const * buffer, bool label) const;

	// actual cursor position
	LyXCursor cursor;

	Selection selection;
	// this is used to handle XSelection events in the right manner
	Selection xsel_cache;

	/// needed for the toggling (cursor position on last selection made)
	LyXCursor last_sel_cursor;
	/// needed for toggling the selection in screen.C
	LyXCursor toggle_cursor;
	/// needed for toggling the selection in screen.C
	LyXCursor toggle_end_cursor;
};

#endif
