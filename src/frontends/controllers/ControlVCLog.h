// -*- C++ -*-
/**
 * \file ControlVCLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLVCLOG_H
#define CONTROLVCLOG_H

#include "Dialog.h"

class ControlVCLog : public Dialog::Controller {
public:
	///
	ControlVCLog(Dialog &);
	/// Nothing to initialise in this case.
	virtual bool initialiseParams(string const &) { return true; }
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }
	/// put the log file into the ostream
	void getVCLogFile(std::ostream & ss) const;
	/// get the filename of the buffer
	string const getBufferFileName() const;
};

#endif // CONTROLVCLOG_H
