// -*- C++ -*-
/**
 * \file GUI.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GUI_H
#define GUI_H

#include "BCView.h"

#include <boost/utility.hpp>

/** This class makes a whole out of the disparate parts of a dialog.
 */
template <typename Controller, typename GUIview,
	  typename Policy,     typename GUIbc>
class GUI : boost::noncopyable {
public:
	///
	GUI(LyXView & lv, Dialogs & d);
	///
	Controller & controller() { return controller_; }
	///
	Controller const & controller() const { return controller_; }
private:
	///
	Controller controller_;
	///
	GUIview view_;
};


template <typename Controller, typename GUIview,
	  typename Policy,     typename GUIbc>
GUI<Controller, GUIview, Policy, GUIbc>::GUI(LyXView & lv, Dialogs & d)
	: controller_(lv, d),
	  view_()
{
	controller_.setView(view_);
	view_.setController(controller_);
	controller_.bc().view(new GUIbc(controller_.bc()));
	controller_.bc().bp(new Policy);
}

#endif // GUI_H
