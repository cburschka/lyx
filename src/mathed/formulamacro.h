// -*- C++ -*-
/**
 * \file formulamacro.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FORMULA_MACRO_H
#define INSET_FORMULA_MACRO_H

#include "formulabase.h"


class MathMacroTemplate;

// An InsetFormulaMacro only knows its name and asks the global
// MathMacroTable if it needs to know more.

/// Main LyX Inset for defining math macros
class InsetFormulaMacro : public InsetFormulaBase {
public:
	///
	InsetFormulaMacro();
	/// construct a macro hull from its name and the number of arguments
	explicit InsetFormulaMacro(std::string const & name, int nargs, std::string const & t);
	/// constructs a mocro from its LaTeX definition
	explicit InsetFormulaMacro(std::string const & s);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void read(Buffer const &, LyXLex & lex);
	///
	void write(Buffer const &, std::ostream & os) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int latex(Buffer const &, std::ostream & os,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream & os,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;

	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	InsetOld::Code lyxCode() const;
	///
	MathAtom const & par() const;
	///
	MathAtom & par();
private:
	///
	void read(std::istream & is);
	/// prefix in inset
	std::string prefix() const;
};

#endif
