/**
 * \file WorkAreaFactory.C
 * Read the file COPYING
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
