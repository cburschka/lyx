// -*- C++ -*-
/**
 * \file ControlForks.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLFORKS_H
#define CONTROLFORKS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"

#include <boost/signals/connection.hpp>

#include <sys/types.h>
#include <vector>

/** A controller for dialogs that display the child processes forked by LyX.
    Also provides an interface enabling them to be killed prematurely.
*/
class ControlForks : public ControlDialogBI {
public:
	///
	ControlForks(LyXView &, Dialogs &);
	///
	std::vector<pid_t> const getPIDs() const;
	///
	string const getCommand(pid_t) const;
	///
	void kill(pid_t);

private:
	///
	virtual void apply();
	/// disconnect from the ForkedcallsController
	virtual void clearParams();
	/// connect to the ForkedcallsController
	virtual void setParams();
	/// Connection to the ForkedcallsController signal
	boost::signals::connection childrenChanged_;
	/// The list of PIDs to kill
	std::vector<string> pids_;
};

#endif // CONTROLFORKS_H
