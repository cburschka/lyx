/**
 * \file LyXScreenFactory.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
 
#include "frontends/LyXScreenFactory.h"

#include "XWorkArea.h"
#include "xscreen.h"
 
namespace LyXScreenFactory {

LyXScreen * create(WorkArea & owner)
{
	return new XScreen(static_cast<XWorkArea &>(owner));
} 
 
}
