/**
 * \file qt2/LyXScreenFactory.C
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

#include "frontends/LyXScreenFactory.h"

#include "QWorkArea.h"
#include "qscreen.h"

namespace LyXScreenFactory {

LyXScreen * create(WorkArea & owner)
{
	return new QScreen(static_cast<QWorkArea &>(owner));
}

}
