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
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlLog.h"
#include "buffer.h"



ControlLog::ControlLog(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlLog::setParams()
{
	logfile_ = buffer()->getLogName();
}


void ControlLog::clearParams()
{
	logfile_.second.erase();
}
