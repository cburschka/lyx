// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlBibtex.h
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLBIBTEX_H
#define CONTROLBIBTEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for Bibtex dialogs.
 */
class ControlBibtex : public ControlCommand
{
public:
	///
	ControlBibtex(LyXView &, Dialogs &);

private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	/// 
	virtual void applyParamsNoInset();
	/// not needed.
	virtual void clearDaughterParams() {}
};

#endif // CONTROLBIBTEX_H
