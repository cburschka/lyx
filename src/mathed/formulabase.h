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

#include <iosfwd>
#include "insets/inset.h"

// only for getType():
#include "math_defs.h"

class Buffer;
class BufferView;
class MathAtom;

///
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
	virtual MathInsetTypes getType() const = 0;

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
	virtual void edit(BufferView *, int x, int y, unsigned int button);
	///
	virtual void edit(BufferView *, bool front = true);
	///
	virtual void toggleInsetCursor(BufferView *);
	///
	virtual void showInsetCursor(BufferView *, bool show = true);
	///
	virtual void hideInsetCursor(BufferView *);
	///
	virtual void getCursorPos(BufferView *, int &, int &) const;
	///
	virtual void toggleInsetSelection(BufferView * bv);
	///
	virtual void insetButtonPress(BufferView *, int x, int y, int button);
	///
	virtual void insetButtonRelease(BufferView *, int x, int y, int button);
	///
	virtual void insetKeyPress(XKeyEvent * ev);
	///
	virtual void insetMotionNotify(BufferView *, int x, int y, int state);
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
	virtual void metrics(BufferView * bv = 0, LyXFont const * font = 0) const;
	///
	virtual void updateLocal(BufferView * bv, bool mark_dirty);
private:
	/// unimplemented
	void operator=(const InsetFormulaBase &);
	///
	mutable BufferView * view_;	
	///
	mutable LyXFont const * font_;
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

#endif
