// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2000 The LyX Team.
 *
 * ======================================================
 *
 * Author: Baruch Even  <baruch@lyx.org>
 */

#ifndef GNOMEBC_H
#define GNOMEBC_H

#include "ButtonController.h"
#include <list>

#ifdef __GNUG__
#pragma interface
#endif

namespace Gtk {
class Button;
class Widget;
}

class gnomeBC : public ButtonControllerBase
{
public:
	///
	gnomeBC(string const & cancel, string const & close);

	/* Initialise Button Functions */
	/// Call refresh() when finished setting the buttons.
	void setOK(Gtk::Button * obj) {
		okay_ = obj;
	}
	///
	void setApply(Gtk::Button * obj) {
		apply_ = obj;
	}
	///
	void setCancel(Gtk::Button * obj) {
		cancel_ = obj;
	}
	///
	void setUndoAll(Gtk::Button * obj) {
		undo_all_ = obj;
	}
	///
	void addReadOnly(Gtk::Widget * obj) {
		read_only_.push_front(obj);
	}
	///
	void eraseReadOnly() {
		read_only_.clear();
	}

	/* Action Functions */
	/// force a refresh of the buttons
	virtual void refresh();

private:
	/// Updates the button sensitivity (enabled/disabled)
	void setSensitive(Gtk::Button * btn, ButtonPolicy::Button id);
	///
	Gtk::Button * okay_;
	///
	Gtk::Button * apply_;
	///
	Gtk::Button * cancel_;
	///
	Gtk::Button * undo_all_;
	/// List of items to be deactivated when in one of the read-only states
	typedef std::list<Gtk::Widget *> WidgetList;
	WidgetList read_only_;
};

#endif // GNOMEBC_H
