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

#include "math_inset.h"
#include "math_macroarg.h"

class MathMacroTemplate;


/** This class contains the data for a macro
    \author Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
    \version November 1996
 */
class MathMacro : public MathInset {
public:
	/// A macro can be built from an existing template
	explicit MathMacro(MathMacroTemplate const &);
	///
	MathMacro(MathMacro const &);
	///
	void draw(Painter &, int, int);
	///
	void Metrics(MathStyles st, int asc = 0, int des = 0);
	///
	MathInset * clone() const;
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
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
	void Validate(LaTeXFeatures &) const;

private:
	///
	MathMacroTemplate const * const tmplate_;
	///
	MathXArray expanded_;
	///
	void operator=(MathMacro const &);
};


inline std::ostream & operator<<(std::ostream & os, MathMacro const & m)
{
	m.dump(os);
	return os;
}
#endif
