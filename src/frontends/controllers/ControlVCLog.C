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
	: ControlConnectBD(lv, d)
{
	d_.showVCLogFile.connect(slot(this, &ControlVCLog::show));
}


void ControlVCLog::show()
{
	if (!lv_.view()->available())
		return;

	logfile_ = lv_.view()->buffer()->lyxvc.getLogFile();

	bc().readOnly(isReadonly());
	view().show();
}


void ControlVCLog::update()
{
	if (!lv_.view()->available())
		return;

	logfile_ = lv_.view()->buffer()->lyxvc.getLogFile();
	
	bc().readOnly(isReadonly());
	view().update();

	lyx::unlink(logfile_); 
}


void ControlVCLog::hide()
{
	logfile_.erase();
	disconnect();
	view().hide();
}
