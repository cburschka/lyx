// -*- C++ -*-
/**
 * \file BCView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef BCVIEW_H
#define BCVIEW_H

#include "LString.h"
#include <boost/shared_ptr.hpp>
#include <list>

class ButtonController;
class ButtonPolicy;

struct CheckedWidget {
	///
	virtual ~CheckedWidget();

	/** Returns true if the widget is in a valid state.
	*  Might also change the visual appearance of the widget,
	*  to reflect this state.
	*/
	virtual bool check() const = 0;
};


class BCView {
public:
	BCView(ButtonController const &);
	///
	virtual ~BCView() {}
	///
	virtual void refresh() = 0;
	///
	virtual void refreshReadOnly() = 0;
	///
	ButtonPolicy & bp() const;
	///
	void addCheckedWidget(CheckedWidget * ptr);
protected:
	///
	bool checkWidgets();

private:
	///
	typedef boost::shared_ptr<CheckedWidget> checked_widget_ptr;
	typedef std::list<checked_widget_ptr> checked_widget_list;
	///
	checked_widget_list checked_widgets;
	///
	ButtonController const & parent;
};


/** A templatised instantiation of the ButtonController's View requiring the
 *  gui-frontend widgets.
 */
template <class Button, class Widget>
class GuiBC : public BCView {
public:
	///
	GuiBC(ButtonController const & parent,
	      string const & cancel, string const & close);

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

	string cancel_label_;
	string close_label_;

	Button * okay_;
	Button * apply_;
	Button * cancel_;
	Button * restore_;

	typedef std::list<Widget *> Widgets;
	Widgets read_only_;
};


#include "BCView.tmpl"

#endif // BCVIEW_H
