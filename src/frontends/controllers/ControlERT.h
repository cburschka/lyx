// -*- C++ -*-
/**
 * \file ControlERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLERT_H
#define CONTROLERT_H


#include "Dialog.h"
#include "insets/insetert.h" // InsetERT::ERTStatus


class ControlERT : public Dialog::Controller {
public:
	///
	ControlERT(Dialog &);
	///
	InsetERT::ERTStatus status() const { return status_; }
	///
	void setStatus(InsetERT::ERTStatus status) { status_ = status; }
	///
	virtual bool initialiseParams(string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
private:
	///
	InsetERT::ERTStatus status_;
};

#endif
