/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlVCLog.C
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ControlVCLog.h"
#include "buffer.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"

using SigC::slot;

ControlVCLog::ControlVCLog(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{
	d_.showVCLogFile.connect(slot(this, &ControlVCLog::show));
}

// FIXME: this is all wrong, getLogFile() actually creates a file
// which we must unlink.

// FIXME: I need to get the Buffer Filename for my window title, need
// to add to params. 

void ControlVCLog::setParams()
{
	logfile_ = lv_.view()->buffer()->lyxvc.getLogFile();
}


void ControlVCLog::clearParams()
{
	logfile_.erase();
}
