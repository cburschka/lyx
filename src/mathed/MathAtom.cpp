/**
 * \file MathAtom.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathAtom.h"
#include "InsetMath.h"

using namespace std;

namespace lyx {


MathAtom::MathAtom()
	: nucleus_(0)
{}


MathAtom::MathAtom(Inset * p)
	: nucleus_(static_cast<InsetMath *>(p))
{}


MathAtom::MathAtom(MathAtom const & at)
	: nucleus_(0)
{
	if (at.nucleus_)
		nucleus_ = static_cast<InsetMath*>(at.nucleus_->clone());
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


} // namespace lyx
