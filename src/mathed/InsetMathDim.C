/**
 * \file InsetMathDim.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathDim.h"

#include "BufferView.h"
#include "coordcache.h"
#include "debug.h"
#include "metricsinfo.h"


namespace lyx {


InsetMathDim::InsetMathDim()
{}


void InsetMathDim::setPosCache(PainterInfo const & pi, int x, int y) const
{
	//lyxerr << "InsetMathDim: cache to " << x << " " << y << std::endl;
	pi.base.bv->coordCache().insets().add(this, x, y);
}


} // namespace lyx
