// -*- C++ -*-
/**
 * \file updatableinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef UPDATABLEINSET_H
#define UPDATABLEINSET_H

//  Updatable Insets. These insets can receive directly user interaction.
//  Note that all pure methods from Inset class are pure here too.
//  [Alejandro 080596]

#include "inset.h"

class UpdatableInset : public InsetOld {
public:
	///
	UpdatableInset() : scx(0) {}
	///
	virtual EDITABLE editable() const;
	/// identification as text inset in a cursor slice
	UpdatableInset * asUpdatableInset() { return this; }

	/// return the cursor dim
	virtual void getCursorDim(int &, int &) const;
	///
	virtual bool showInsetDialog(BufferView *) const { return false; }

protected:
	/// scrolls offset pixels
	void setScroll(int maxwidth, int offset) const;
	/// returns the actual scroll value
	virtual int scroll(bool recursive = true) const;
	///
	mutable int scx;
};

#endif
