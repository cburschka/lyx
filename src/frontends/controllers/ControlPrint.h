// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *          Copyright 2001 The LyX Team.
 *
 *======================================================
 *
 * \file ControlPrint.h
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef CONTROLPRINT_H
#define CONTROLPRINT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"

class PrinterParams;

/** A controller for Print dialogs.
 */
class ControlPrint : public ControlDialogBD {
public:
	///
	ControlPrint(LyXView &, Dialogs &);

	/// Browse for a file
	string const Browse(string const &);
	///
	PrinterParams & params() const;

private:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// set the params before show or update.
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();

	///
	PrinterParams * params_;
};

#endif // CONTROLPRINT_H
