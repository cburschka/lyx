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

#include "Dialog.h"
#include "buffer.h" // Buffer::LogType
#include <utility>

/**
 * A controller for a read-only text browser.
 */
class ControlLog : public Dialog::Controller {
public:
	///
	ControlLog(Dialog &);
	///
	virtual bool initialiseParams(string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }
	///
	std::pair<Buffer::LogType, string> const & logfile() const {
		return logfile_;
	}
private:

	std::pair<Buffer::LogType, string> logfile_;
};

#endif // CONTROLLOG_H
