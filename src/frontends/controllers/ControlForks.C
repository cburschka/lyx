/** 
 * \file ControlForks.C
 * Copyright 2001 The LyX Team
 * Read COPYING
 *
 * \author Angus Leeming
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlForks.h"
#include "ButtonControllerBase.h"
#include "ViewBase.h"

#include "BufferView.h"
#include "LyXView.h"
#include "lyxfunc.h"

#include "frontends/Dialogs.h"

#include "support/forkedcontr.h"
#include "support/lstrings.h"

using std::vector;
using SigC::slot;

ControlForks::ControlForks(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{
	d_.showForks.connect(slot(this, &ControlForks::show));
}


vector<pid_t> const ControlForks::getPIDs() const
{
	ForkedcallsController const & fcc = ForkedcallsController::get();
	return fcc.getPIDs();
}


string const ControlForks::getCommand(pid_t pid) const
{
	ForkedcallsController const & fcc = ForkedcallsController::get();
	return fcc.getCommand(pid);
}


void ControlForks::kill(pid_t pid)
{
	pids_.push_back(tostr(pid));
}


void ControlForks::apply()
{
	if (!lv_.view()->available())
		return;

	view().apply();

	// Nothing to apply?
	if (pids_.empty())
		return;

	for (vector<string>::const_iterator it = pids_.begin();
	     it != pids_.end(); ++it) {
		lv_.getLyXFunc()->dispatch(LFUN_FORKS_KILL, *it);
	}

	pids_.clear();
}


void ControlForks::setParams()
{
	if (childrenChanged_.connected())
		return;

	pids_.clear();

	ForkedcallsController & fcc = ForkedcallsController::get();
	childrenChanged_ =
		fcc.childrenChanged.connect(slot(this, &ControlForks::update));
}


void ControlForks::clearParams()
{
	pids_.clear();
	childrenChanged_.disconnect();
}

