// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
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
