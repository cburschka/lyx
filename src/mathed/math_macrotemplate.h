// -*- C++ -*-
#ifndef MATH_MACROTEMPLATE_H
#define MATH_MACROTEMPLATE_H

#include "math_nestinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacro;

/** This class contains the macro definition
    \author Alejandro Aguilar Sierra
 */
//class MathMacroTemplate : public MathInset, boost::noncopyable

class MathMacroTemplate : public MathNestInset {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(string const & name, int nargs,
		MathArray const & = MathArray(), MathArray const & = MathArray());
	///
	explicit MathMacroTemplate(std::istream & is);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	/// Number of arguments
	int numargs() const;
	///
	void numargs(int);
	///
	string name() const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metrics(MathMetricsInfo & st) const;
	/// identifies macro templates
	MathMacroTemplate * asMacroTemplate() { return this; }
	/// identifies macro templates
	MathMacroTemplate const * asMacroTemplate() const { return this; }
private:
	///
	int numargs_;
	///
	string name_;
};

#endif
