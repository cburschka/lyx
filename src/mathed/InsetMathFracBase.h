// -*- C++ -*-
/**
 * \file InsetMathFracBase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FRACBASE_H
#define MATH_FRACBASE_H

#include "InsetMathNest.h"


namespace lyx {


class InsetMathFracBase : public InsetMathNest {
public:
	///
	explicit InsetMathFracBase(idx_type ncells = 2);
	///
	bool idxUpDown(Cursor &, bool up) const;
	///
	bool idxLeft(Cursor &) const;
	///
	bool idxRight(Cursor &) const;
};



} // namespace lyx
#endif
