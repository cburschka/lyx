// -*- C++ -*-
/**
 * \file ControlChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */
 
#ifndef CONTROLCHANGES_H
#define CONTROLCHANGES_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"

/**
 * A controller for the merge changes dialog.
 */
class ControlChanges : public ControlDialogBD {
public:
	ControlChanges(LyXView &, Dialogs &);

	/// find the next merge chunk and highlight it
	void find();

	/// return date of change
	string const getChangeDate();

	/// return author of change
	string const getChangeAuthor();
 
	/// accept the current merge
	void accept();

	/// reject the current merge
	void reject();

private:
	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLCHANGES_H
