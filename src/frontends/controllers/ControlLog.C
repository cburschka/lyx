/**
 * \file ControlLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
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
