/**
 * \file xforms/WorkAreaFactory.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/WorkAreaFactory.h"
#include "XWorkArea.h"


namespace WorkAreaFactory {

WorkArea * create(LyXView & owner, int w, int h)
{
	return new XWorkArea(owner, w, h);
}

}
