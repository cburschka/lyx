// -*- C++ -*-
/*
 *  File:        formulabase.h
 *  Purpose:     Common parts of the math LyX insets
 *  Author:      André Pönitz
 *  Created:     May 2001
 *  Description: Allows the edition of math paragraphs inside Lyx.
 *
 *  Copyright: 2001, The LyX Project
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef INSET_FORMULABASE_H
#define INSET_FORMULABASE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insets/inset.h"
#include "frontends/mouse_state.h"
#include "lyxfont.h"

#include <boost/weak_ptr.hpp>

#include <iosfwd>

class Buffer;
class BufferView;
class MathAtom;

/// An abstract base class for all math related LyX insets
class InsetFormulaBase : public UpdatableInset {
public:
	///
	InsetFormulaBase();
	///
	virtual int ascent(BufferView *, LyXFont const &) const = 0;
	///
	virtual int descent(BufferView *, LyXFont const &) const = 0;
	///
	virtual int width(BufferView *, LyXFont const &) const = 0;
	///
	virtual void draw(BufferView *,LyXFont const &, int, float &, bool) const = 0;
	///
	virtual string hullType() const { return "none"; }
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
	virtual void validate(LaTeXFeatures &) const;
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const = 0;
	///
	virtual Inset::Code lyxCode() const;
	/// what appears in the minibuffer when opening
	virtual string const editMessage() const;
	///
	virtual void edit(BufferView *, int x, int y, mouse_button::state button);
	///
	virtual void edit(BufferView *, bool front = true);
	///
	virtual void toggleInsetCursor(BufferView *);
	///
	virtual void showInsetCursor(BufferView *, bool show = true);
	///
	virtual void hideInsetCursor(BufferView *);
	///
	virtual void fitInsetCursor(BufferView *) const;
	///
	virtual void getCursorPos(BufferView *, int &, int &) const;
	///
	virtual void toggleInsetSelection(BufferView * bv);
	///
	virtual void insetButtonPress(BufferView *, int x, int y, mouse_button::state button);
	///
	virtual bool insetButtonRelease(BufferView *, int x, int y, mouse_button::state button);
	///
	virtual void insetMotionNotify(BufferView *, int x, int y, mouse_button::state state);
	///
	virtual void insetUnlock(BufferView *);

	/// To allow transparent use of math editing functions
	virtual RESULT localDispatch(BufferView *, kb_action, string const &);

	///
	virtual std::vector<string> const getLabelList() const;
	///
	virtual MathAtom const & par() const = 0;
	///
	virtual MathAtom & par() = 0;
	///
	virtual void updateLocal(BufferView * bv, bool mark_dirty);
	///
	BufferView * view() const { return view_.get(); }

	///
	virtual bool searchForward(BufferView *, string const &,
	                           bool = true, bool = false);
	///
	virtual bool searchBackward(BufferView *, string const &,
	                            bool = true, bool = false);
	///
	virtual bool isTextInset() const { return true; }
	///
	virtual void mutateToText();
	///
	virtual void revealCodes(BufferView *) const;
	///
	virtual Inset::EDITABLE editable() const { return HIGHLY_EDITABLE; }

private:
	/// unimplemented
	void operator=(const InsetFormulaBase &);
	/// common base for handling accents
	void handleAccent(BufferView * bv, string const & arg, string const & name);

protected:
	///
	mutable boost::weak_ptr<BufferView> view_;
	///
	mutable LyXFont font_;

protected:
	///
	void metrics(BufferView * bv, LyXFont const & font) const;
	///
	void metrics(BufferView * bv = 0) const;
	///
	void handleFont(BufferView * bv, string const & arg, string const & font);

	///
	mutable int xo_;
	///
	mutable int yo_;
};

// We don't really mess want around with mathed stuff outside mathed.
// So do it here.
//
void mathDispatchCreation(BufferView *, string const &, bool);
//
void mathDispatchMathDisplay(BufferView *, string const &);
//
void mathDispatchMathMode(BufferView *, string const &);
//
void mathDispatchMathMacro(BufferView *, string const &);
//
void mathDispatchMathDelim(BufferView *, string const &);
//
void mathDispatchInsertMath(BufferView *, string const &);
//
void mathDispatchInsertMatrix(BufferView *, string const &);
//
void mathDispatchGreek(BufferView *, string const &);
//
void mathDispatchMathImportSelection(BufferView *, string const &);
//
void mathDispatch(BufferView *, kb_action, string const &);
//
void mathDispatch(BufferView *, string const &);

#endif
