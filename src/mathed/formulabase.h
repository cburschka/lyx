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
	virtual void Write(Buffer const *, std::ostream &) const = 0;
	///
	virtual void Read(Buffer const *, LyXLex & lex) = 0;
	///
	virtual int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const = 0;
	///
	virtual int Ascii(Buffer const *, std::ostream &, int linelen) const = 0;
	///
	virtual int Linuxdoc(Buffer const *, std::ostream &) const = 0;
	///
	virtual int DocBook(Buffer const *, std::ostream &) const = 0;
	///
	virtual void Validate(LaTeXFeatures &) const;
	///
	virtual Inset * Clone(Buffer const &) const = 0;
	///
	virtual Inset::Code LyxCode() const;
	///
	virtual LyXFont const ConvertFont(LyXFont const & f) const;
	/// what appears in the minibuffer when opening
	virtual string const EditMessage() const;
	///
	virtual void Edit(BufferView *, int x, int y, unsigned int button);
	///
	virtual void ToggleInsetCursor(BufferView *);
	///
	virtual void ShowInsetCursor(BufferView *, bool show = true);
	///
	virtual void HideInsetCursor(BufferView *);
	///
	virtual void GetCursorPos(BufferView *, int &, int &) const;
	///
	virtual void ToggleInsetSelection(BufferView * bv);
	///
	virtual void InsetButtonPress(BufferView *, int x, int y, int button);
	///
	virtual void InsetButtonRelease(BufferView *, int x, int y, int button);
	///
	virtual void InsetKeyPress(XKeyEvent * ev);
	///
	virtual void InsetMotionNotify(BufferView *, int x, int y, int state);
	///
	virtual void InsetUnlock(BufferView *);
   
	///  To allow transparent use of math editing functions
	virtual RESULT LocalDispatch(BufferView *, kb_action, string const &);
    
	///
	virtual std::vector<string> const getLabelList() const;
	///
	MathInset * par() const;
protected:
	///
	virtual void UpdateLocal(BufferView * bv);

	///
	MathInset * par_;
};

#endif
