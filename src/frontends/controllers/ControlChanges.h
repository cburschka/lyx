// -*- C++ -*-
/**
 * \file ControlChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLCHANGES_H
#define CONTROLCHANGES_H

#include "Dialog.h"

/**
 * A controller for the merge changes dialog.
 */
class ControlChanges : public Dialog::Controller {
public:
	///
	ControlChanges(Dialog &);
	/// Nothing to initialise in this case.
	virtual bool initialiseParams(std::string const &) { return true; }
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }

	/// find the next merge chunk and highlight it
	bool find();

	/// return date of change
	std::string const getChangeDate();

	/// return author of change
	std::string const getChangeAuthor();

	/// accept the current merge
	void accept();

	/// reject the current merge
	void reject();
};

#endif // CONTROLCHANGES_H
