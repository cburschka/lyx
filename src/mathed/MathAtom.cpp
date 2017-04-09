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


MathAtom::MathAtom(InsetMath * p)
	: nucleus_(p)
{}


MathAtom::MathAtom(MathAtom const & at)
	: nucleus_(at.nucleus_ ? static_cast<InsetMath*>(at->clone()) : nullptr)
{}


MathAtom & MathAtom::operator=(MathAtom const & at)
{
	// copy then move-assign
	return operator=(MathAtom(at));
}


} // namespace lyx
