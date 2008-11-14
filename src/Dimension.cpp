/**
 * \file Dimension.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Dimension.h"

namespace lyx {

void Dimension::operator+=(Dimension const & dim)
{
	if (asc < dim.asc)
		asc = dim.asc;
	if (des < dim.des)
		des = dim.des;
	wid += dim.wid;
}

} // namespace lyx
