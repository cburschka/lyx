/**
 * \file qt2/WorkAreaFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "frontends/WorkAreaFactory.h"

#include "QWorkArea.h"

namespace WorkAreaFactory {

WorkArea * create(int x, int y, int w, int h)
{
	return new QWorkArea(x, y, w, h);
}

} // namespace WorkAreaFactory
