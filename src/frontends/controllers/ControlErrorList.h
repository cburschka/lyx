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


#include "Dialog.h"
#include <vector>



/** A controller for the ErrorList dialog.
 */


class ControlErrorList : public Dialog::Controller {
public:
	/// A class to hold an error item
	struct ErrorItem {
		std::string error;
		std::string description;
		int par_id;
		int pos_start;
		int pos_end;
		ErrorItem(string const &, string const &, int, int, int);
	};
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

	/// get the current item
	int currentItem() const;
	/// goto this error in the parent bv
	void goTo(int item);
	/// return the parent document name
	string const & docName();
	/// rescan the log file and rebuild the error list
	void fillErrors();
	/// clear everything
	void clearErrors();
	///
	std::vector<ErrorItem> const & ErrorList() const;
private:
	///
	std::vector<ErrorItem> ErrorList_;
	///
	string logfilename_;
	///
	int current_;
};

#endif // CONTROLERRORLIST_H
