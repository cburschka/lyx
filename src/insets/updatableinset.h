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
	/// check if the font of the char we want inserting is correct
	/// and modify it if it is not.
	virtual bool checkInsertChar(LyXFont &) { return true; }
	///
	virtual EDITABLE editable() const;

	/// return the cursor pos, relative to the inset pos
	virtual void getCursorPos(BufferView *, int &, int &) const {}
	/// return the cursor dim
	virtual void getCursorDim(BufferView *, int &, int &) const;
	///
	virtual bool insertInset(BufferView *, InsetOld *) { return false; }
	///
	virtual int insetInInsetY() const { return 0; }
	// We need this method to not clobber the real method in Inset
	int scroll(bool recursive = true) const
		{ return InsetOld::scroll(recursive); }
	///
	virtual bool showInsetDialog(BufferView *) const { return false; }
	///
	virtual void toggleSelection(BufferView *, bool /*kill_selection*/) {}

protected:
	///  An updatable inset could handle lyx editing commands
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
	/// scrolls to absolute position in bufferview-workwidth * sx units
	void scroll(BufferView *, float sx) const;
	/// scrolls offset pixels
	void scroll(BufferView *, int offset) const;
};

#endif
