// -*- C++ -*-

/*
 *  Main LyX Inset for defining math macros
 *
 *  \author Alejandro Aguilar Sierra
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS
 *
 *  You are free to use and modify this code under the terms of
 *  the GNU General Public Licence version 2 or later.
 */

#ifndef INSET_FORMULA_MACRO_H
#define INSET_FORMULA_MACRO_H

#include "formulabase.h"


class MathMacroTemplate;

// An InsetFormulaMacro only knows its name and asks the global
// MathMacroTable if it needs to know more.

///
class InsetFormulaMacro : public InsetFormulaBase {
public:
	///
	InsetFormulaMacro();
	/// construct a macro hull from its name and the number of arguments
	explicit InsetFormulaMacro(string const & name, int nargs);
	/// constructs a mocro from its LaTeX definition
	explicit InsetFormulaMacro(string const & s);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void read(Buffer const *, LyXLex & lex);
	///
	void write(Buffer const *, std::ostream & os) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int latex(Buffer const *, std::ostream & os,
		  LatexRunParams const &) const;
	///
	int linuxdoc(Buffer const *, std::ostream & os) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;

	///
	Inset * clone(Buffer const &) const;
	///
	Inset::Code lyxCode() const;
	///
	MathAtom const & par() const;
	///
	MathAtom & par();
private:
	///
	void read(std::istream & is);
	/// prefix in inset
	string prefix() const;
};

#endif
