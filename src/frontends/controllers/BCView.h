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
 * Full author contact details are available in file CREDITS.
 */

#ifndef BCVIEW_H
#define BCVIEW_H

#include "support/docstring.h"

#include <boost/shared_ptr.hpp>
#include <list>

namespace lyx {
namespace frontend {

class ButtonController;
class ButtonPolicy;


/** \c CheckedWidget is an abstract base class that can be stored
 *  in the button controller's view and can be interrogated by it
 *  when the activation state of the Ok, Apply buttons is refreshed.
 *  Ideally, the user will be prevented from returning invalid data
 *  to the LyX kernel.
 *
 *  Many widgets can be grouped together in the derived class if they
 *  make a logical whole. E.g., an input and a choice widget that together
 *  are used to set a Length can be interrogated together.
 */
class CheckedWidget {
public:
	///
	virtual ~CheckedWidget() {}

	/** Returns true if the widget is in a valid state.
	*  Might also change the visual appearance of the widget,
	*  to reflect this state.
	*/
	virtual bool check() const = 0;
};


/** \c BCView is the View to ButtonController's Controller. It
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


} // namespace frontend
} // namespace lyx

#endif // BCVIEW_H
