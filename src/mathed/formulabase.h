// -*- C++ -*-
/*
 *  File:        formula.h
 *  Purpose:     Declaration of formula inset
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Allows the edition of math paragraphs inside Lyx. 
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *  Version: 0.4, Lyx project.
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
#include <boost/smart_ptr.hpp>

#include "insets/inset.h"

class Buffer;
class MathInset;

///
class InsetFormulaBase : public UpdatableInset {
public:
	/// 
	InsetFormulaBase(InsetFormulaBase const & p);
	///
	explicit InsetFormulaBase(MathInset *);
	///
	virtual ~InsetFormulaBase();
	///
	virtual int ascent(BufferView *, LyXFont const &) const = 0;
	///
	virtual int descent(BufferView *, LyXFont const &) const = 0;
	///
	virtual int width(BufferView *, LyXFont const &) const = 0;
	///
	virtual void draw(BufferView *,LyXFont const &, int, float &, bool) const = 0;
	///
	virtual void write(Buffer const *, std::ostream &) const = 0;
	///
	virtual void read(Buffer const *, LyXLex & lex) = 0;
	///
	virtual int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const = 0;
	///
	virtual int ascii(Buffer const *, std::ostream &, int linelen) const = 0;
	///
	virtual int linuxdoc(Buffer const *, std::ostream &) const = 0;
	///
	virtual int docBook(Buffer const *, std::ostream &) const = 0;
	///
	virtual void validate(LaTeXFeatures &) const;
	///
	virtual Inset * clone(Buffer const &) const = 0;
	///
	virtual Inset::Code lyxCode() const;
	///
	virtual LyXFont const convertFont(LyXFont const & f) const;
	/// what appears in the minibuffer when opening
	virtual string const editMessage() const;
	///
	virtual void edit(BufferView *, int x, int y, unsigned int button);
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
   
	///  To allow transparent use of math editing functions
	virtual RESULT localDispatch(BufferView *, kb_action, string const &);
    
	///
	virtual std::vector<string> const getLabelList() const;
	///
	MathInset * par() const;
protected:
	///
	virtual void updateLocal(BufferView * bv);

	///
	MathInset * par_;
};

#endif
