// -*- C++ -*-
/*
 *  File:        math_macro.h
 *  Purpose:     Declaration of macro class for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     November 1996
 *  Description: WYSIWYG math macros
 *
 *  Dependencies: Math
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.2, Math & Lyx project.
 *
 *   This code is under the GNU General Public Licence version 2 or later.
 */
#ifndef MATH_MACRO_H
#define MATH_MACRO_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include <iosfwd>

#include "math_nestinset.h"
#include "math_macroarg.h"

class MathMacroTemplate;


/** This class contains the data for a macro
    \author Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
    \version November 1996
 */
class MathMacro : public MathNestInset {
public:
	/// A macro can be built from an existing template
	explicit MathMacro(MathMacroTemplate const &);
	///
	MathMacro(MathMacro const &);
	///
	void draw(Painter &, int x, int y) const;
	///
	void metrics(MathStyles st) const;
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void dump(std::ostream & os) const;

	///
	bool idxUp(int &, int &) const;
	///
	bool idxDown(int &, int &) const;
	///
	bool idxLeft(int &, int &) const;
	///
	bool idxRight(int &, int &) const;

	///
	void validate(LaTeXFeatures &) const;

private:
	///
	void operator=(MathMacro const &);
	///
	string const & name() const;

	///
	MathMacroTemplate const * const tmplate_;
	///
	mutable MathXArray expanded_;
};


inline std::ostream & operator<<(std::ostream & os, MathMacro const & m)
{
	m.dump(os);
	return os;
}
#endif
