/**
 * \file ControlPreamble.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef CONTROLPREAMBLE_H
#define CONTROLPREAMBLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for Preamble dialogs.
 */
class ControlPreamble : public ControlDialog<ControlConnectBD> {
public:
	///
	ControlPreamble(LyXView &, Dialogs &);

	///
	string & params() const;

private:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// set the params before show or update.
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();
    
	///
	string * params_;
};

#endif // CONTROLPREAMBLE_H
