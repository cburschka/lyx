// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ButtonController.h
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author Baruch Even, baruch.even@writeme.com
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include <list>

#include "gettext.h"
#include "ButtonControllerBase.h"

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


template <class Button, class Widget>
GuiBC<Button, Widget>::GuiBC(string const & cancel, string const & close)
	: ButtonControllerBase(cancel, close),
	  okay_(0), apply_(0), cancel_(0), restore_(0)
{}


template <class Button, class Widget>
void GuiBC<Button, Widget>::refresh()
{
	if (okay_) {
		bool const enabled = bp().buttonStatus(ButtonPolicy::OKAY);
		setButtonEnabled(okay_, enabled);
	}
	if (apply_) {
		bool const enabled = bp().buttonStatus(ButtonPolicy::APPLY);
		setButtonEnabled(apply_, enabled);
	}
	if (restore_) {
		bool const enabled = bp().buttonStatus(ButtonPolicy::RESTORE);
		setButtonEnabled(restore_, enabled);
	}
	if (cancel_) {
		bool const enabled = bp().buttonStatus(ButtonPolicy::CANCEL);
		if (enabled)
			setButtonLabel(cancel_, cancel_label_);
		else
			setButtonLabel(cancel_, close_label_);
	}
}


template <class Button, class Widget>
void GuiBC<Button, Widget>::refreshReadOnly()
{
	if (read_only_.empty()) return;

	bool const enable = !bp().isReadOnly();

	Widgets::const_iterator end = read_only_.end();
	Widgets::const_iterator iter = read_only_.begin();
	for (; iter != end; ++iter) {
		setWidgetEnabled(*iter, enable);
	}
}


template <class BP, class GUIBC>
class ButtonController: public GUIBC
{
public:
	///
	ButtonController(string const & = _("Cancel"),
			 string const & = _("Close"));
	///
	virtual ButtonPolicy & bp() { return bp_; }

protected:
	///
	BP bp_;
};


template <class BP, class GUIBC>
ButtonController<BP, GUIBC>::ButtonController(string const & cancel,
					      string const & close)
	: GUIBC(cancel, close)
{}

#endif // BUTTONCONTROLLER_H
