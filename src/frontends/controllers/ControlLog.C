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

#include "ControlLog.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "BufferView.h"

#include "frontends/LyXView.h"

#include <boost/bind.hpp>

ControlLog::ControlLog(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{
	d_.showLogFile = boost::bind(&ControlLog::show, this);
}


void ControlLog::setParams()
{
	logfile_ = lv_.view()->buffer()->getLogName();
}


void ControlLog::clearParams()
{
	logfile_.second.erase();
}
