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
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"

using SigC::slot;

ControlIndex::ControlIndex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INDEX_INSERT)
{
	d_.showIndex.connect(slot(this, &ControlIndex::showInset));
	d_.createIndex.connect(slot(this, &ControlIndex::createInset));
}
