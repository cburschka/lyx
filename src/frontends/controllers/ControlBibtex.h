// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlBibtex.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLBIBTEX_H
#define CONTROLBIBTEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"
#include "ButtonController.h"
#include "ViewBase.h"

/** A controller for Bibtex dialogs.
 */
class ControlBibtex : public ControlCommand
{
public:
	///
	ControlBibtex(LyXView &, Dialogs &);

protected:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
};

/** This class instantiates and makes available the GUI-specific
    ButtonController and View.
 */
template <class GUIview, class GUIbc>
class GUIBibtex : public ControlBibtex {
public:
	///
	GUIBibtex(LyXView &, Dialogs &);
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
GUIBibtex<GUIview, GUIbc>::GUIBibtex(LyXView & lv, Dialogs & d)
	: ControlBibtex(lv, d),
	  view_(*this)
{}

#endif // CONTROLBIBTEX_H
