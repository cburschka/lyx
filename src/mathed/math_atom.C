/*
 *  File:        math_atom.C
 *  Purpose:     Wrapper for MathInset * 
 *  Author:      André Pönitz
 *  Created:     July 2001
 *
 *  Copyright: 2001 The LyX team
 *
 *   Version: 1.2.0
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_atom.h"
#include "math_inset.h"
#include "support/LAssert.h"

#include <utility>


using std::swap;


MathAtom::MathAtom()
	: nucleus_(0)
{}


MathAtom::MathAtom(MathInset * p)
	: nucleus_(p)
{}


MathAtom::MathAtom(MathAtom const & p)
	: nucleus_(p.nucleus_ ? p.nucleus_->clone() : 0)
{}


void MathAtom::operator=(MathAtom const & p)
{
	if (&p == this)
		return;
	MathAtom tmp(p);
	swap(tmp.nucleus_, nucleus_);
}


MathAtom::~MathAtom()
{
	delete nucleus_;
}


void MathAtom::reset(MathInset * p)
{
	if (p == nucleus_)
		return;
	delete nucleus_;
	nucleus_ = p;
}


MathInset * MathAtom::nucleus() const
{
	lyx::Assert(nucleus_);
	return nucleus_;
}


MathInset * MathAtom::operator->() const
{
	return nucleus();
}
