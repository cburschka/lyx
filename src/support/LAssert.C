/**
 * \file LAssert.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LAssert.h"

#ifdef ENABLE_ASSERTIONS
#include "lyx_main.h"

void emergencyCleanup() {
	static bool didCleanup;
	if (didCleanup)
		return;

	didCleanup = true;

	LyX::emergencyCleanup();
}

#endif
