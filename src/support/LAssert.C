/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

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
