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

#include <utility>


MathAtom::MathAtom()
	: nucleus_(0)
{}


MathAtom::MathAtom(MathInset * p)
	: nucleus_(p)
{}


MathAtom::MathAtom(MathAtom const & at)
	: nucleus_(at.nucleus_ ? at.nucleus_->clone() : 0)
{}


void MathAtom::operator=(MathAtom const & at)
{
	if (&at == this)
		return;
	MathAtom tmp(at);
	std::swap(tmp.nucleus_, nucleus_);
}


MathAtom::~MathAtom()
{
	delete nucleus_;
}
