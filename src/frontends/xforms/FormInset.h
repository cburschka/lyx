// -*- C++ -*-
/**
 * \file FormInset.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

/* A base class for dialogs connected to insets. This class is temporary in that
 * it has been superceeded by the controller-view split.
 */

#ifndef FORMCOMMAND_H
#define FORMCOMMAND_H

#include "FormBaseDeprecated.h"
#include "insets/insetcommand.h"

#ifdef __GNUG__
#pragma interface
#endif


/** This class is an XForms GUI base class to insets
 */
class FormInset : public FormBaseBD {
protected:
	/// Constructor
	FormInset(LyXView *, Dialogs *, string const &);

	/// Connect signals. Also perform any necessary initialisation.
	virtual void connect();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// bool indicates if a buffer switch took place
	virtual void updateSlot(bool);

	/// inset::hide connection.
	SigC::Connection ih_;
};


/** This class is an XForms GUI base class to insets derived from
    InsetCommand
 */
class FormCommand : public FormInset {
protected:
	/// Constructor
	FormCommand(LyXView *, Dialogs *, string const &);

	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Slot launching dialog to (possibly) create a new inset
	void createInset(string const &);
	/// Slot launching dialog to an existing inset
	void showInset(InsetCommand *);

	/// pointer to the inset passed through showInset
	InsetCommand * inset_;
	/// the nitty-griity. What is modified and passed back
	InsetCommandParams params;
};

#endif
