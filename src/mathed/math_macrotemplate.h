// -*- C++ -*-
#ifndef MATH_MACROTEMPLATE_H
#define MATH_MACROTEMPLATE_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMacro;

/** This class contains the macro definition
    \author Alejandro Aguilar Sierra
 */
//class MathMacroTemplate : public MathInset, boost::noncopyable 

class MathMacroTemplate : public MathInset {
public:
	///
	MathMacroTemplate();
	///
	MathMacroTemplate(string const & name, int nargs);
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	/// Number of arguments
	int numargs() const;
	///
	void numargs(int);
	///
	void draw(Painter &, int, int);
	///
	void metrics(MathStyles st);
private:
	///
	int numargs_;
	/// unimplemented
	void operator=(MathMacroTemplate const &);
};

#endif
