// -*- C++ -*-
/*
 *  File:        math_root.h
 *  Purpose:     Declaration of the root object
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     January 1999
 *  Description: Root math object
 *
 *  Copyright: 1999 Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_ROOT_H
#define MATH_ROOT_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** The general n-th root inset.
    \author Alejandro Aguilar Sierra
    \version January 1999
 */
class MathRootInset : public MathNestInset {
public:
	///
	MathRootInset();
	///
	MathInset * clone() const;
	///
	bool idxUpDown(idx_type & idx, bool up) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
};

#endif
