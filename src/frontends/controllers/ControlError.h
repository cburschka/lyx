// -*- C++ -*-
/**
 * \file ControlError.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLERROR_H
#define CONTROLERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"
#include "insets/inseterror.h" // needed for proper instantiation of GUI<>.

class InsetError;

/** A controller for LaTeX Error dialogs.
 */
class ControlError : public ControlInset<InsetError, string>
{
public:
	///
	ControlError(LyXView &, Dialogs &);

private:
	/// not needed.
	virtual void applyParamsToInset() {}
	///
	virtual void applyParamsNoInset() {}
	/// get the parameters from the string passed to createInset.
	virtual string const getParams(string const &) { return string(); }
	/// get the parameters from the inset passed to showInset.
	virtual string const getParams(InsetError const &);
};

#endif // CONTROLERROR_H
