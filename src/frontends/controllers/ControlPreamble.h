// -*- C++ -*-
/**
 * \file ControlPreamble.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLPREAMBLE_H
#define CONTROLPREAMBLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"

#include "LString.h"

/** A controller for Preamble dialogs.
 */
class ControlPreamble : public ControlDialogBD {
public:
	///
	ControlPreamble(LyXView &, Dialogs &);

	///
	string const & params() const;
	///
	void params(string const & newparams);
private:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// set the params before show or update.
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();

	///
	string params_;
};

#endif // CONTROLPREAMBLE_H
