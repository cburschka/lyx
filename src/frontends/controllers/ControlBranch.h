// -*- C++ -*-
/**
 * \file ControlBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLBRANCH_H
#define CONTROLBRANCH_H


#include "Dialog.h"
#include "debug.h"

class InsetBranchParams;

class ControlBranch : public Dialog::Controller {
public:
	///
	ControlBranch(Dialog &);
	///
	virtual bool initialiseParams(string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetBranchParams & params() { return *params_.get(); }
	///
	InsetBranchParams const & params() const { return *params_.get(); }
	///
private:
	///
	boost::scoped_ptr<InsetBranchParams> params_;
};

#endif // CONTROLBRANCH_H
