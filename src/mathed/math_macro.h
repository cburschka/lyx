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

#include "math_nestinset.h"
#include "math_metricsinfo.h"
#include "math_macroarg.h"
#include "LString.h"

class MathMacroTemplate;


/** This class contains the data for a macro
    \author Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
    \version November 1996
 */
class MathMacro : public MathNestInset {
public:
	/// A macro can be built from an existing template
	explicit MathMacro(string const &);
	///
	MathMacro(MathMacro const &);
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	MathInset * clone() const;
	///
	void dump() const;

	///
	bool idxUpDown(idx_type & idx, pos_type & pos, bool up, int targetx) const;
	///
	bool idxLeft(idx_type & idx, pos_type & pos) const;
	///
	bool idxRight(idx_type & idx, pos_type & pos) const;

	///
	void validate(LaTeXFeatures &) const;
	///
	bool isMacro() const { return true; }
	///
	bool match(MathInset *) const { return false; }

	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;

private:
	///
	void operator=(MathMacro const &);
	///
	string name() const;
	///
	bool defining() const;
	///
	void updateExpansion() const;
	///
	void expand() const;

	///
	MathAtom & tmplate_;
	///
	mutable MathArray expanded_;
	///
	mutable MathMetricsInfo mi_;
	///
	mutable LyXFont font_;
};


#endif
