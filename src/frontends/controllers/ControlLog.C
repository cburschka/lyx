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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ButtonController.h"
#include "ControlLog.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "ViewBase.h"

using std::make_pair;
using SigC::slot;

ControlLog::ControlLog(LyXView & lv, Dialogs & d)
	: ControlConnectBD(lv, d)
{
	d_.showLogFile.connect(slot(this, &ControlLog::show));
}


void ControlLog::show()
{
	if (!lv_.view()->available())
		return;

	logfile_ = lv_.view()->buffer()->getLogName();

	bc().readOnly(isReadonly());
	view().show();
}


void ControlLog::update()
{
	if (!lv_.view()->available())
		return;

	logfile_ = lv_.view()->buffer()->getLogName();
	
	bc().readOnly(isReadonly());
	view().update();
}


void ControlLog::hide()
{
	logfile_.second.erase();
	disconnect();
	view().hide();
}
