// -*- C++ -*-
/**
 * \file insetbutton.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

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
