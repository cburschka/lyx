/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlLog.h
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlLog.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "BufferView.h"

using SigC::slot;

ControlLog::ControlLog(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{
	d_.showLogFile.connect(slot(this, &ControlLog::show));
}


void ControlLog::setParams()
{
	logfile_ = lv_.view()->buffer()->getLogName();
}


void ControlLog::clearParams()
{
	logfile_.second.erase();
}
