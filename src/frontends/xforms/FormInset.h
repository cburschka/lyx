// -*- C++ -*-
/**
 * \file FormInset.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

/* A base class for dialogs connected to insets. This class is temporary in that
 * it has been superceeded by the controller-view split.
 */

#ifndef FORMCOMMAND_H
#define FORMCOMMAND_H

#include "FormBaseDeprecated.h"
#include "insets/insetcommandparams.h"

#include <boost/signals/connection.hpp>

#ifdef __GNUG__
#pragma interface
#endif

class InsetCommand;

/** This class is an XForms GUI base class to insets
 */
class FormInset : public FormBaseBD {
protected:
	/// Constructor
	FormInset(LyXView &, Dialogs &, string const &);

	/// Connect signals. Also perform any necessary initialisation.
	virtual void connect();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// bool indicates if a buffer switch took place
	virtual void updateSlot(bool);

	/// inset::hide connection.
	boost::signals::connection ih_;
};

#endif
