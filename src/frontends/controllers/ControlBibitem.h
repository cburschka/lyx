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
 * \file ControlBibitem.h
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLBIBITEM_H
#define CONTROLBIBITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for Bibitem dialogs.
 */
class ControlBibitem : public ControlCommand {
public:
	///
	ControlBibitem(LyXView &, Dialogs &);

private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	/// not needed.
	virtual void applyParamsNoInset() {}
};

#endif // CONTROLBIBITEM_H
