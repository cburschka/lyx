// -*- C++ -*-
/**
 * \file ControlLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLLOG_H
#define CONTROLLOG_H

#include <utility>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "buffer.h" // Buffer::LogType

/**
 * A controller for a read-only text browser.
 */
class ControlLog : public ControlDialogBD {
public:
	///
	ControlLog(LyXView &, Dialogs &);
	///
	std::pair<Buffer::LogType, string> const & logfile()
		{ return logfile_; }

private:
	///
	virtual void apply() {}
	/// set the params before show or update
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();

	std::pair<Buffer::LogType, string> logfile_;
};

#endif // CONTROLLOG_H
