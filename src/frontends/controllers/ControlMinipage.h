// -*- C++ -*-
/**
 * \file ControlMinipage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLMINIPAGE_H
#define CONTROLMINIPAGE_H

#include "Dialog.h"
#include "insets/insetminipage.h"

class ControlMinipage : public Dialog::Controller {
public:
	///
	ControlMinipage(Dialog &);
	///
	virtual void initialiseParams(string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetMinipage::Params & params() { return *params_.get(); }
	///
	InsetMinipage::Params const & params() const { return *params_.get(); }
private:
	///
	boost::scoped_ptr<InsetMinipage::Params> params_;
};

#endif
