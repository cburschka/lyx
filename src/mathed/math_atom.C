/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_atom.h"
#include "math_inset.h"
#include "support/LAssert.h"


MathAtom::MathAtom()
	: nucleus_(0)
{}


MathAtom::MathAtom(MathInset * p)
	: nucleus_(p)
{}


MathAtom::MathAtom(MathAtom const & p)
{
	copy(p);
}


void MathAtom::operator=(MathAtom const & p)
{
	if (this != &p) {
		done();
		copy(p);
	}
}


MathAtom::~MathAtom()
{
	done();
}


void MathAtom::reset(MathInset * p)
{
	done();
	nucleus_ = p;
}



void MathAtom::done()
{
	delete nucleus_;
}


void MathAtom::copy(MathAtom const & p)
{
	//cerr << "calling MathAtom::copy\n";
	nucleus_   = p.nucleus_;
	if (nucleus_)
		nucleus_ = nucleus_->clone();
}


MathInset * MathAtom::nucleus() const
{
	lyx::Assert(nucleus_);
	return nucleus_;
}


bool MathAtom::hasNucleus() const
{
	return nucleus_;
}


MathInset * MathAtom::operator->() const
{
	return nucleus();
}
