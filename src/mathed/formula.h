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

#ifndef INSET_FORMULA_H 
#define INSET_FORMULA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "mathed/formulabase.h"
#include "math_defs.h"

class MathMatrixInset;

///
class InsetFormula : public InsetFormulaBase {
public:
	///
	InsetFormula();
	///
	explicit InsetFormula(MathInsetTypes);
	///
	explicit InsetFormula(string const &);
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;

	///
	void write(std::ostream &) const;
	///
	void read(LyXLex & lex);
	///
	int latex(std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(std::ostream &, int linelen) const;
	///
	int linuxdoc(std::ostream &) const;
	///
	int docBook(std::ostream &) const;

	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const;
	///
	virtual RESULT localDispatch(BufferView *, kb_action, string const &);
	///
	std::vector<string> const getLabelList() const;
	///
	void handleExtern(string const & arg, BufferView * bv);
	///
	MathMatrixInset * par() const;
	///
	bool display() const;
	///
	bool ams() const;
private:
	/// Safe setting of contents
	void par(MathInset *);
};
#endif
