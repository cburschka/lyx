/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LAssert.h"

#ifdef ENABLE_ASSERTIONS
#include "bufferlist.h"

extern BufferList bufferlist;

void emergencySave() {
	static bool didSafe = false;
	if (didSafe)
		return;

	didSafe = true;

	// emergency save
	if (!bufferlist.empty())
		bufferlist.emergencyWriteAll();
}

#endif
