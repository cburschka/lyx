// -*- C++ -*-
/*
 * \file ButtonController.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming <leeming@lyx.org>
 * \author Baruch Even, baruch.even@writeme.com
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H


#include "ButtonControllerBase.h"
#include "gettext.h"
#include <list>

/** A templatised instantiation of the ButtonController requiring the
 *  gui-frontend widgets.
 *  The template declarations are in ButtonController.tmpl, which should
 *  be #included in the gui-frontend BC class, see e.g. xforms/xformsBC.C
 */
template <class Button, class Widget>
class GuiBC : public ButtonControllerBase
{
public:
	///
	GuiBC(string const & cancel, string const & close);

	///
	void setOK(Button * obj) { okay_ = obj; }
	///
	void setApply(Button * obj) { apply_ = obj; }
	///
	void setCancel(Button * obj) { cancel_ = obj; }
	///
	void setRestore(Button * obj) { restore_ = obj; }
	///
	void addReadOnly(Widget * obj) { read_only_.push_back(obj); }
	///
	void eraseReadOnly() { read_only_.clear(); }

	/// Refresh the status of the Ok, Apply, Restore, Cancel buttons.
	void refresh();
	/// Refresh the status of any widgets in the read_only list
	void refreshReadOnly();

private:
	/// Enable/Disable a widget
	virtual void setWidgetEnabled(Widget * obj, bool enable) = 0;
	/// Enable/Disable a button
	virtual void setButtonEnabled(Button * obj, bool enable) = 0;
	/// Set the Label on the button
	virtual void setButtonLabel(Button * obj, string const & label) = 0;

	Button * okay_;
	Button * apply_;
	Button * cancel_;
	Button * restore_;

	typedef std::list<Widget *> Widgets;
	Widgets read_only_;
};


template <class BP, class GUIBC>
class ButtonController: public GUIBC
{
public:
	///
	ButtonController(string const & = _("Cancel"),
			 string const & = _("Close"));
	///
	~ButtonController() {}
	///
	virtual ButtonPolicy & bp() { return bp_; }

protected:
	///
	BP bp_;
};


#include "ButtonController.tmpl"

#endif // BUTTONCONTROLLER_H
