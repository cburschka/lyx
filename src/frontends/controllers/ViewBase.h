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
 * Author: Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef VIEWBASE_H
#define VIEWBASE_H

#include <boost/utility.hpp>
#include "ControlButton.h"
#include "ControlSplash.h"

class ViewBase {
public:
	/// 
	ViewBase(ControlButton & c) : controller_(c) {}
	/// 
	virtual ~ViewBase() {}

	/// Apply changes to LyX data from dialog.
	virtual void apply() = 0;
	/// Hide the dialog.
	virtual void hide() = 0;
	/// Redraw the dialog (e.g. if the colors have been remapped).
	virtual void redraw() {}
	/// Create the dialog if necessary, update it and display it.
	virtual void show() = 0;
	/// Update dialog before/whilst showing it.
	virtual void update() = 0;

	/** These shortcuts allow (e.g. xform's) global callback functions
	    access to the buttons without making the whole controller_ public.
	*/
        ///
        void ApplyButton() { controller_.ApplyButton(); }
        ///
        void OKButton() { controller_.OKButton(); }
        ///
        void CancelButton() { controller_.CancelButton(); }
        ///
        void RestoreButton() { controller_.RestoreButton(); }

protected:
	/// The view is, after all, controlled!
	ControlButton & controller_;
};


class ViewSplash {
public:
	/// 
	ViewSplash(ControlSplash & c) : controller_(c) {}
	/// 
	virtual ~ViewSplash() {}

	/// Hide the dialog.
	virtual void hide() = 0;
	/// Create the dialog and show it.
	virtual void show() = 0;

	/** The shortcut allows (e.g. xform's) global callback functions
	    access without making the whole controller_ public.
	*/
        ///
        void Hide() { controller_.hide(); }

protected:
	/// The view is, after all, controlled!
	ControlSplash & controller_;
};


/** A generic class to cast the ButtonController controller_.bc_ to it's
    daughter class. */
template <class GUIbc>
class ViewBC : public ViewBase {
public:
	///
	ViewBC(ControlButton & c) : ViewBase(c) {}

protected:
	///
	GUIbc & bc() const
	{
		return static_cast<GUIbc &>(controller_.bc());
		// return dynamic_cast<GUIbc &>(controller_.bc());
	}
};

#endif // VIEWBASE_H
