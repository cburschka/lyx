/**
 * \file LyXScreenFactory.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "frontends/LyXScreenFactory.h"

#include "QWorkArea.h"
#include "qscreen.h"

namespace LyXScreenFactory {

LyXScreen * create(WorkArea & owner)
{
	return new QScreen(static_cast<QWorkArea &>(owner));
}

}
