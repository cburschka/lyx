/**
 * \file WorkAreaFactory.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
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
