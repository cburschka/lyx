// -*- C++ -*-
/**
 * \file ViewBase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef VIEWBASE_H
#define VIEWBASE_H

class ControlButtons;
class ButtonController;

#include <boost/utility.hpp>

class ViewBase : boost::noncopyable {
public:
	///
	ViewBase();
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
	///
	virtual bool isVisible() const = 0;

	/** Defaults to nothing. Can be used by the controller, however, to
	 *  indicate to the view that something has changed and that the
	 *  dialog therefore needs updating.
	 */
	virtual void partialUpdate(int) {}

	/** This should be set by the GUI class that owns both the controller
	 *  and the view
	 */
	void setController(ControlButtons &);

	///
	ControlButtons & getController();
	///
	ControlButtons const & getController() const;
	///
	ButtonController & bc();
protected:
	/// We don't own this.
	ControlButtons * controller_ptr_;
};

#endif // VIEWBASE_H
