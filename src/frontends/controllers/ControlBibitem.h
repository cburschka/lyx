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
#include "ButtonController.h"
#include "ViewBase.h"

/** A controller for Bibitem dialogs.
 */
class ControlBibitem : public ControlCommand
{
public:
	///
	ControlBibitem(LyXView &, Dialogs &);

protected:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
};

/** This class instantiates and makes available the GUI-specific
    ButtonController and View.
 */
template <class GUIview, class GUIbc>
class GUIBibitem : public ControlBibitem {
public:
	///
	GUIBibitem(LyXView &, Dialogs &);
	///
	virtual ButtonControllerBase & bc() { return bc_; }
	///
	virtual ViewBase & view() { return view_; }

private:
	///
	ButtonController<OkCancelReadOnlyPolicy, GUIbc> bc_;
	///
	GUIview view_;
};

template <class GUIview, class GUIbc>
GUIBibitem<GUIview, GUIbc>::GUIBibitem(LyXView & lv, Dialogs & d)
	: ControlBibitem(lv, d),
	  view_(*this)
{}

#endif // CONTROLBIBITEM_H
