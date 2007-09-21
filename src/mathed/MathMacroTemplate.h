// -*- C++ -*-
/**
 * \file math_macrotemplate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTEMPLATE_H
#define MATH_MACROTEMPLATE_H

#include "MathData.h"
#include "MacroTable.h"
#include "InsetMathNest.h"

#include "support/types.h"


namespace lyx {

/// This class contains the macro definition.
class MathMacroTemplate : public InsetMathNest {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(docstring const & name, int nargs,
		docstring const & type,
		MathData const & = MathData(),
		MathData const & = MathData());
	///
	explicit MathMacroTemplate(const docstring & str);
	///
	void edit(Cursor & cur, bool left);
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	void read(Buffer const &, Lexer & lex);
	///
	void write(Buffer const &, std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;

	/// Number of arguments
	int numargs() const;
	///
	void numargs(int);
	///
	docstring name() const;
	///
	MacroData asMacroData() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// identifies macro templates
	MathMacroTemplate * asMacroTemplate() { return this; }
	/// identifies macro templates
	MathMacroTemplate const * asMacroTemplate() const { return this; }
	///
	Inset::Code lyxCode() const { return MATHMACRO_CODE; }

private:
	virtual Inset * clone() const;
	/// prefix in inset
	docstring prefix() const;

	///
	int numargs_;
	///
	docstring name_;
	/// newcommand or renewcommand or def
	docstring type_;
};


} // namespace lyx

#endif
