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

//  Updatable Insets. These insets can be locked and receive
//  directly user interaction. Currently used only for mathed.
//  Note that all pure methods from Inset class are pure here too.
//  [Alejandro 080596]

#include "inset.h"

#include "support/types.h"


/** Extracted from Matthias notes:
 *
 * An inset can simple call LockInset in it's edit call and *ONLY*
 * in it's edit call.
 *
 * Unlocking is either done by LyX or the inset itself with a
 * UnlockInset-call
 *
 * During the lock, all button and keyboard events will be modified
 * and send to the inset through the following inset-features. Note that
 * InsetOld::insetUnlock will be called from inside UnlockInset. It is meant
 * to contain the code for restoring the menus and things like this.
 *
 * If a inset wishes any redraw and/or update it just has to call
 * updateInset(this).
 *
 * It's is completly irrelevant, where the inset is. UpdateInset will
 * find it in any paragraph in any buffer.
 * Of course the_locking_inset and the insets in the current paragraph/buffer
 *  are checked first, so no performance problem should occur.
 */
class UpdatableInset : public InsetOld {
public:
	///
	UpdatableInset();
	///
	UpdatableInset(UpdatableInset const & in);

	/// check if the font of the char we want inserting is correct
	/// and modify it if it is not.
	virtual bool checkInsertChar(LyXFont &) { return true; }
	///
	virtual EDITABLE editable() const;

	///
	virtual void fitInsetCursor(BufferView *) const;
	/// FIXME
	virtual void getCursorPos(BufferView *, int &, int &) const {}
	/// Get the absolute document x,y of the cursor
	virtual void getCursor(BufferView &, int &, int &) const = 0;
	///
	virtual void insetUnlock(BufferView *);
	///
	virtual void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual bool insertInset(BufferView *, InsetOld *) { return false; }
	///
	virtual UpdatableInset * getLockingInset() const
		{ return const_cast<UpdatableInset *>(this); }
	///
	virtual UpdatableInset * getFirstLockingInsetOfType(InsetOld::Code c)
		{ return (c == lyxCode()) ? this : 0; }
	///
	virtual int insetInInsetY() const { return 0; }
	///
	virtual bool lockInsetInInset(BufferView *, UpdatableInset *)
		{ return false; }
	///
	virtual bool unlockInsetInInset(BufferView *, UpdatableInset *,
					bool /*lr*/ = false)
		{ return false; }
	///  An updatable inset could handle lyx editing commands
	virtual RESULT localDispatch(FuncRequest const & cmd);
	// We need this method to not clobber the real method in Inset
	int scroll(bool recursive = true) const
		{ return InsetOld::scroll(recursive); }
	///
	virtual bool showInsetDialog(BufferView *) const { return false; }
	///
	// needed for spellchecking text
	///
	virtual bool allowSpellcheck() const { return false; }
	///
	virtual WordLangTuple const
	selectNextWordToSpellcheck(BufferView *, float & value) const;
	///
	virtual void selectSelectedWord(BufferView *) {}
	///
	virtual void toggleSelection(BufferView *, bool /*kill_selection*/) {}

	/// find the next change in the inset
	virtual bool nextChange(BufferView * bv, lyx::pos_type & length);

	///
	// needed for search/replace functionality
	///
	virtual bool searchForward(BufferView *, std::string const &,
				   bool = true, bool = false);
	///
	virtual bool searchBackward(BufferView *, std::string const &,
				    bool = true, bool = false);

protected:
	/// scrolls to absolute position in bufferview-workwidth * sx units
	void scroll(BufferView *, float sx) const;
	/// scrolls offset pixels
	void scroll(BufferView *, int offset) const;
};

#endif
