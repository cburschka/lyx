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

#include "math_data.h"
#include "math_macrotable.h"
#include "math_nestinset.h"


/// This class contains the macro definition.
class MathMacroTemplate : public MathNestInset {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(std::string const & name, int nargs,
		std::string const & type,
		MathArray const & = MathArray(),
		MathArray const & = MathArray());
	///
	explicit MathMacroTemplate(std::istream & is);
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	void edit(LCursor & cur, bool left);
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	void read(Buffer const &, LyXLex & lex);
	///
	void write(Buffer const &, std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	/// Number of arguments
	int numargs() const;
	///
	void numargs(int);
	///
	std::string name() const;
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
	InsetBase::Code lyxCode() const { return MATHMACRO_CODE; }

private:
	/// prefix in inset
	std::string prefix() const;

	///
	int numargs_;
	///
	std::string name_;
	/// newcommand or renewcommand or def
	std::string type_;
};

#endif
