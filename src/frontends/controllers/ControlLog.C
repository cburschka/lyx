/**
 * \file ControlLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlLog.h"


ControlLog::ControlLog(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlLog::initialiseParams(string const &)
{
	logfile_ = kernel().buffer()->getLogName();
	return true;
}


void ControlLog::clearParams()
{
	logfile_.second.erase();
}
