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

#include "formulabase.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacroTemplate;

// InsetFormulaMacro's ParInset is the ParInset of the macro definition
// which in turn is stored in the global MathMacroTable.
// No copying/updating needed anymore...

///
class InsetFormulaMacro: public InsetFormulaBase {
public:
	///
	explicit InsetFormulaMacro();
	///
	explicit InsetFormulaMacro(string name, int na);
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *,LyXFont const &, int, float &, bool) const;

	///
	void read(LyXLex & lex);
	///
	void write(std::ostream & os) const;
	///
	int ascii(std::ostream &, int linelen) const;
	///
	int latex(std::ostream & os, bool fragile, bool free_spc) const;
	///
	int linuxdoc(std::ostream & os) const;
	///
	int docBook(std::ostream &) const;

	///
	Inset * clone(Buffer const &) const;
	///
	Inset::Code lyxCode() const;
	///
	RESULT localDispatch(BufferView *, kb_action, string const &);
private:
	/// prefix in inset
	string prefix() const;
	///
	MathMacroTemplate * tmacro() const;
};

#endif
