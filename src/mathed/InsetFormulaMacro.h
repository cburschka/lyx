// -*- C++ -*-
/**
 * \file InsetFormulaMacro.h
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

#include "InsetMathNest.h"


namespace lyx {

class MathMacroTemplate;
class Lexer;


// An InsetFormulaMacro only knows its name and asks the global
// MathMacroTable if it needs to know more.

/// Main LyX Inset for defining math macros
class InsetFormulaMacro : public InsetMathNest {
public:
	///
	InsetFormulaMacro();
	/// construct a macro hull from its name and the number of arguments
	InsetFormulaMacro(docstring const & name, int nargs, docstring const & t);
	/// constructs a mocro from its LaTeX definition
	explicit InsetFormulaMacro(docstring const & s);
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void read(Buffer const &, Lexer & lex);
	///
	void write(Buffer const &, std::ostream & os) const;
	///
	int latex(Buffer const &, odocstream & os,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;

	///
	Inset::Code lyxCode() const { return MATHMACRO_CODE; }
	///
	docstring const & getInsetName() const { return name_; }
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
private:
	///
	MathAtom & tmpl() const;
	///
	void read(std::istream & is);
	/// prefix in inset
	docstring prefix() const;
	///
	docstring name_;
	///
	Inset * clone() const;
};


} // namespace lyx

#endif
