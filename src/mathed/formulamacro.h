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

#ifndef INSET_FORMULA_MACRO_H 
#define INSET_FORMULA_MACRO_H

#include "formula.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacroTemplate;

// InsetFormulaMacro's ParInset is the ParInset of the macro definition
// which in turn is stored in the global MathMacroTable.
// No copying/updating needed anymore...

///
class InsetFormulaMacro: public InsetFormula {
public:
	///
	InsetFormulaMacro();
	///
	explicit
	InsetFormulaMacro(string name, int na);
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *,LyXFont const &, int, float &, bool) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	void Write(Buffer const *, std::ostream & os) const;
	///
	int Latex(Buffer const *, std::ostream & os, bool fragile,
		  bool free_spc) const;
	///
	int Linuxdoc(Buffer const *, std::ostream & os) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	Inset * Clone(Buffer const &) const;
	///
	Inset::Code LyxCode() const;
	/// what appears in the minibuffer when opening
	string const EditMessage() const;
	///
	RESULT LocalDispatch(BufferView *, kb_action, string const &);
private:
	/// prefix in inset
	string prefix() const;
	///
	MathMacroTemplate * tmacro() const;
};

inline Inset::Code InsetFormulaMacro::LyxCode() const
{
	return Inset::MATHMACRO_CODE;
}
#endif
