// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 2000-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_BUTTON_H
#define INSET_BUTTON_H

#ifdef __GNUG__
#pragma interface
#endif

#include "inset.h"
#include "LString.h"

/** Used to provide an inset that looks like a button.
 */
class InsetButton: public Inset {
public:
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;

protected:
	/// This should provide the text for the button
	virtual string const getScreenLabel(Buffer const *) const = 0;
};

#endif

