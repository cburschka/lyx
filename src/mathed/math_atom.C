/**
 * \file math_atom.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_atom.h"
#include "math_inset.h"

using std::swap;


MathAtom::MathAtom()
	: nucleus_(0)
{}


MathAtom::MathAtom(InsetBase * p)
	: nucleus_(static_cast<MathInset *>(p))
{}


MathAtom::MathAtom(MathAtom const & at)
	: nucleus_(0)
{
	if (at.nucleus_)
		nucleus_ = static_cast<MathInset*>(at.nucleus_->clone().release());
}


MathAtom & MathAtom::operator=(MathAtom const & at)
{
	if (&at == this)
		return *this;
	MathAtom tmp(at);
	swap(tmp.nucleus_, nucleus_);
	return *this;
}


MathAtom::~MathAtom()
{
	delete nucleus_;
}
