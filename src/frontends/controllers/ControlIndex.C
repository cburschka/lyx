/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlIndex.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlIndex.h"


ControlIndex::ControlIndex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INDEX_INSERT)
{}
