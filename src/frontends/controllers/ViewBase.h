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

#include "support/LAssert.h"

#include <boost/utility.hpp>

class ControlButtons;


class ViewBase : boost::noncopyable {
public:
	///
	ViewBase() : controller_ptr_(0) {}
	///
	virtual ~ViewBase() {}

	/// Apply changes to LyX data from dialog.
	virtual void apply() = 0;
	/// build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	virtual void hide() = 0;
	/// Redraw the dialog (e.g. if the colors have been remapped).
	virtual void redraw() {}
	/// Create the dialog if necessary, update it and display it.
	virtual void show() = 0;
	/// Update dialog before/whilst showing it.
	virtual void update() = 0;

	/** Defaults to nothing. Can be used by the controller, however, to
	 *  indicate to the view that something has changed and that the
	 *  dialog therefore needs updating.
	 */
	virtual void partialUpdate(int) {}

	/** This should be set by the GUI class that owns both the controller
	 *  and the view
	 */
	void setController(ControlButtons & c) { controller_ptr_ = &c; }
	
	///
	ControlButtons & getController()
	{
		lyx::Assert(controller_ptr_);
		return *controller_ptr_;
	}
	///
	ControlButtons const & getController() const
	{
		lyx::Assert(controller_ptr_);
		return *controller_ptr_;
	}

protected:
	/// We don't own this.
	ControlButtons * controller_ptr_;
};


#endif // VIEWBASE_H
