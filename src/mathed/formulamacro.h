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

// InsetFormulaMacro's only knows its name and asks the global
// MathMacroTable if it needs to know more. 

///
class InsetFormulaMacro: public InsetFormulaBase {
public:
	///
	InsetFormulaMacro();
	/// construct a macro hull from its name and the number of arguments
	explicit InsetFormulaMacro(string name, int na);
	/// constructs a mocro from its LaTeX definition
	explicit InsetFormulaMacro(string const & s);
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
	int docbook(std::ostream &) const;

	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const;
	///
	RESULT localDispatch(BufferView *, kb_action, string const &);
	///
	MathInsetTypes getType() const;
	///
	MathInset const * par() const;
	///
	void metrics() const;
private:
	///
	MathMacroTemplate const & tmacro() const;
	/// prefix in inset
	string prefix() const;
};

#endif
