// -*- C++ -*-
/**
 * \file ControlErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLERRORLIST_H
#define CONTROLERRORLIST_H

#include "errorlist.h"
#include "Dialog.h"
#include <vector>



/** A controller for the ErrorList dialog.
 */


class ControlErrorList : public Dialog::Controller {
public:
	///
	ControlErrorList(Dialog & parent);
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual bool initialiseParams(const string & params);
	///
	virtual void ControlErrorList::clearParams();
	///
	virtual void ControlErrorList::dispatchParams() {}

	/// goto this error in the parent bv
	void goTo(int item);
	/// return the parent document name
	string const & name();
	///
	ErrorList const & errorList() const;
private:
	///
	ErrorList errorlist_;
	///
	string name_;
};

#endif // CONTROLERRORLIST_H
