// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_BUTTON_H
#define INSET_BUTTON_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"

/** Used to provide an inset that looks like a button.
 */
class InsetButton: public Inset {
public:
	///
	InsetButton();

	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;

protected:
	/// This should provide the text for the button
	virtual string getScreenLabel() const = 0;
};

#endif

