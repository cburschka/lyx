/**
 * \file xforms/LyXScreenFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/LyXScreenFactory.h"

#include "xscreen.h"
#include "XWorkArea.h"


namespace LyXScreenFactory {

LyXScreen * create(WorkArea & owner)
{
	using lyx::frontend::XScreen;
	using lyx::frontend::XWorkArea;

	return new XScreen(static_cast<XWorkArea &>(owner));
}

}
