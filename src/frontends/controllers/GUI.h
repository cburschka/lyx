// -*- C++ -*-
/**
 * \file GUI.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef GUI_H
#define GUI_H

#include "ButtonController.h"


/** This class makes a whole out of the disparate parts of a dialog.
 */
template <typename Controller, typename GUIview,
	  typename Policy,     typename GUIbc>
class GUI {
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
	ButtonController<Policy, GUIbc> bc_;
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
	controller_.setButtonController(bc_);
	view_.setController(controller_);
}


#endif // GUI_H
