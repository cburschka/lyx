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

#include "math_nestinset.h"
#include "support/std_string.h"


class MathMacro;

//class MathMacroTemplate : public MathInset, boost::noncopyable

/// This class contains the macro definition.
class MathMacroTemplate : public MathNestInset {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(string const & name, int nargs, string const & type,
		MathArray const & = MathArray(), MathArray const & = MathArray());
	///
	explicit MathMacroTemplate(std::istream & is);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void write(WriteStream & os) const;
	/// Number of arguments
	int numargs() const;
	///
	void numargs(int);
	///
	string name() const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// identifies macro templates
	MathMacroTemplate * asMacroTemplate() { return this; }
	/// identifies macro templates
	MathMacroTemplate const * asMacroTemplate() const { return this; }
private:
	///
	int numargs_;
	///
	string name_;
	/// newcommand or renewcommand or def
	string type_;
};

#endif
