// -*- C++ -*-
/**
 * \file InsetMathDim.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DIMINSET_H
#define MATH_DIMINSET_H

#include <boost/assert.hpp>
#include "InsetMath.h"

namespace lyx {

class PainterInfo;


/// things that need the dimension cache
class InsetMathDim : public InsetMath {
public:
	///
	InsetMathDim();

	///
	Dimension dimensions() const { return dim_; }
	///
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;

	///
	void setPosCache(PainterInfo const & pi, int x, int y) const;
	///
	int plaintext(Buffer const &, odocstream & os,
	              OutputParams const &) const
		{ // FIXME: math macros are output here but
		  // should be handled in InsetMathHull
		  os << "[math macro]"; return 12; }
};


} // namespace lyx

#endif
