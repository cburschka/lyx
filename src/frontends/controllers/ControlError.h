// -*- C++ -*-
/**
 * \file ControlError.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLERROR_H
#define CONTROLERROR_H


#include "Dialog.h"


class ControlError : public Dialog::Controller {
public:
	///
	ControlError(Dialog &);
	///
	virtual void initialiseParams(string const &);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }
	///
	string & params() { return params_; }
	///
	string const & params() const { return params_; }
	///
private:
	string params_;
};

#endif // CONTROLERROR_H
