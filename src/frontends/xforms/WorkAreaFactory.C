/**
 * \file WorkAreaFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
 
#include "frontends/WorkAreaFactory.h"
 
#include "XWorkArea.h"
 
namespace WorkAreaFactory {
 
WorkArea * create(int x, int y, int w, int h)
{
	return new XWorkArea(x, y, w, h);
}
 
}
