// -*- C++ -*-
/**
 * \file ControlVCLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLVCLOG_H
#define CONTROLVCLOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"

#include "LString.h"

/**
 * A controller for the Version Control log viewer.
 */
class ControlVCLog : public ControlDialogBD {
public:
	///
	ControlVCLog(LyXView &, Dialogs &);
	/// put the log file into the ostream
	void getVCLogFile(std::ostream & ss) const;
	/// get the filename of the buffer
	string const getBufferFileName() const;
private:
	///
	virtual void apply() {}
};

#endif // CONTROLVCLOG_H
