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
	virtual EDITABLE editable() const;
	/// identification as text inset in a cursor slice
	UpdatableInset * asUpdatableInset() { return this; }

	/// return the cursor dim
	virtual void getCursorDim(int &, int &) const;
	// We need this method to not clobber the real method in Inset
	int scroll(bool recursive = true) const
		{ return InsetOld::scroll(recursive); }
	///
	virtual bool showInsetDialog(BufferView *) const { return false; }

protected:
	///  An updatable inset could handle lyx editing commands
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
	/// scrolls to absolute position in bufferview-workwidth * sx units
	void scroll(BufferView &, float sx) const;
	/// scrolls offset pixels
	void scroll(BufferView &, int offset) const;
};

#endif
