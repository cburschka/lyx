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


/** \class CheckedWidget is an abstract base class that can be stored
 *  in the button controller's view and can be interrogated by it
 *  when the activation state of the Ok, Apply buttons is refreshed.
 *  Ideally, the user will be prevented from returning invalid data
 *  to the LyX kernel.
 *
 *  Many widgets can be grouped together in the derived class if they
 *  make a logical whole. E.g., an input and a choice widget that together
 *  are used to set a LyXLength can be interrogated together. 
 */
struct CheckedWidget {
	///
	virtual ~CheckedWidget();

	/** Returns true if the widget is in a valid state.
	*  Might also change the visual appearance of the widget,
	*  to reflect this state.
	*/
	virtual bool check() const = 0;
};


/** \class BCView is the View to ButtonController's Controller. It
 *  stores the individual GUI widgets and sets their activation state
 *  upon receipt of instructions from the controller.
 *
 *  It is a base class. The true, GUI, instantiations derive from it.
 */
class BCView {
public:
	BCView(ButtonController const &);
	virtual ~BCView() {}

	//@{
	/// Refresh the status of the Ok, Apply, Restore, Cancel buttons.
	virtual void refresh() const = 0;
	/// Refresh the status of any widgets in the read_only list
	virtual void refreshReadOnly() const = 0;
	//@}

	/// A shortcut to the BP of the BC.
	ButtonPolicy & bp() const;

	/** Add a widget to the list of all widgets whose validity should
	 *  be checked explicitly when the buttons are refreshed.
	 */
	void addCheckedWidget(CheckedWidget * ptr);

protected:
	/// \return true if all CheckedWidgets are in a valid state.
	bool checkWidgets() const;

private:
	typedef boost::shared_ptr<CheckedWidget> checked_widget_ptr;
	typedef std::list<checked_widget_ptr> checked_widget_list;
	checked_widget_list checked_widgets;
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

	//@{
	/** Store pointers to these widgets. The pointers are _not_
	 *  owned by GuiBC.
	 */
	void setOK(Button * obj) { okay_ = obj; }
	void setApply(Button * obj) { apply_ = obj; }
	void setCancel(Button * obj) { cancel_ = obj; }
	void setRestore(Button * obj) { restore_ = obj; }
	//@}

	/** Add a pointer to the list of widgets whose activation
	 *  state is dependent upon the read-only status of the
	 *  underlying buffer.
	 */
	void addReadOnly(Widget * obj) { read_only_.push_back(obj); }

	/// Refresh the status of the Ok, Apply, Restore, Cancel buttons.
	virtual void refresh() const;
	/// Refresh the status of any widgets in the read_only list
	virtual void refreshReadOnly() const;

private:
	/// Enable/Disable a widget
	virtual void setWidgetEnabled(Widget * obj, bool enable) const = 0;
	/// Enable/Disable a button
	virtual void setButtonEnabled(Button * obj, bool enable) const = 0;
	/// Set the Label on the button
	virtual void setButtonLabel(Button * obj, string const & label) const = 0;

	string const cancel_label_;
	string const close_label_;

	Button * okay_;
	Button * apply_;
	Button * cancel_;
	Button * restore_;

	typedef std::list<Widget *> Widgets;
	Widgets read_only_;
};


#include "BCView.tmpl"

#endif // BCVIEW_H
