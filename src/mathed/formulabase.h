// -*- C++ -*-
/**
 * \file formulabase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 *
 * Common parts of the math LyX insets.
 */

#ifndef INSET_FORMULABASE_H
#define INSET_FORMULABASE_H

#include "insets/updatableinset.h"

class Buffer;
class BufferView;
class MathAtom;
class CursorSlice;
class LCursor;


/// An abstract base class for all math related LyX insets
class InsetFormulaBase : public UpdatableInset {
public:
	///
	InsetFormulaBase();
	/// lowest x coordinate
	virtual int xlow() const;
	/// highest x coordinate
	virtual int xhigh() const;
	/// lowest y coordinate
	virtual int ylow() const;
	/// highest y coordinate
	virtual int yhigh() const;

public:
	///
	// Don't use this for AMS validation as long as there is no
	// user-accessible way to override "false positives"
	virtual void validate(LaTeXFeatures &) const;
	///
	virtual InsetOld::Code lyxCode() const;
	/// what appears in the minibuffer when opening
	virtual std::string const editMessage() const;
	/// get the absolute document x,y of the cursor
	virtual void getCursorPos(BufferView & bv, int & x, int & y) const;
	///
	virtual void getCursorDim(int &, int &) const;
	///
	virtual void insetUnlock(BufferView & bv);

	/// To allow transparent use of math editing functions
	//virtual void status(FuncRequest const &);

	///
	virtual MathAtom const & par() const = 0;
	///
	virtual MathAtom & par() = 0;
	///
	///
	virtual bool searchForward(BufferView *, std::string const &,
				   bool = true, bool = false);
	///
	virtual bool searchBackward(BufferView *, std::string const &,
				    bool = true, bool = false);
	///
	virtual bool isTextInset() const { return true; }
	///
	virtual void mutateToText();
	///
	virtual void revealCodes(BufferView & bv) const;
	///
	virtual EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	bool display() const;
	// return the selection as std::string
	std::string selectionAsString(BufferView & bv) const;
	///
	void edit(BufferView * bv, bool);
	///
	void edit(BufferView * bv, int, int);
protected:
	/// To allow transparent use of math editing functions
	virtual
	DispatchResult priv_dispatch(BufferView & bv, FuncRequest const & cmd);
private:
	/// unimplemented
	void operator=(const InsetFormulaBase &);
	/// common base for handling accents
	void handleAccent(BufferView & bv, std::string const & arg, std::string const & name);
	/// lfun handler
	DispatchResult lfunMousePress(BufferView &, FuncRequest const &);
	///
	DispatchResult lfunMouseRelease(BufferView &, FuncRequest const &);
	///
	DispatchResult lfunMouseMotion(BufferView &, FuncRequest const &);

protected:

	/** Find the PreviewLoader, add a LaTeX snippet to it and
	 *  start the loading process.
	 *
	 *  Most insets have no interest in this capability, so the method
	 *  defaults to empty.
	 */
	virtual void generatePreview(Buffer const &) const {}

	///
	void handleFont(LCursor &, std::string const & arg, std::string const & font);
	///
	void handleFont2(LCursor &, std::string const & arg);
};

// We don't really mess want around with mathed stuff outside mathed.
// So do it here.
void mathDispatch(BufferView & bv, FuncRequest const & cmd);

///
void releaseMathCursor(BufferView & bv);

#endif
