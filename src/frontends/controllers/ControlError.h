// -*- C++ -*-
/*
 * \file ControlError.h
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef CONTROLERROR_H
#define CONTROLERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"

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
