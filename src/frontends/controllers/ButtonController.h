// -*- C++ -*-
/**
 * \file ButtonController.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H


#include "ButtonPolicies.h"

#include "LString.h"
#include <boost/scoped_ptr.hpp>


/** Controls the activation of the OK, Apply and Cancel buttons.
 *
 * Actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 * Author: Allan Rae <rae@lyx.org>.
 * This class stripped of xforms-specific code by
 * Angus Leeming <leeming@lyx.org>
 */
class BCView;

class ButtonController : boost::noncopyable {
public:
	///
	~ButtonController();

	///
	BCView & view() const;
	///
	void view(BCView *);

	///
	ButtonPolicy & bp() const;
	///
	void bp(ButtonPolicy *);

	///
	void input(ButtonPolicy::SMInput);
	///
	void ok();
	///
	void apply();
	///
	void cancel();
	///
	void restore();
	///
	void hide();

	///
	void refresh();
	///
	void refreshReadOnly();

	/// Passthrough function -- returns its input value
	bool readOnly(bool = true);
	///
	void readWrite();

	///
	void valid(bool = true);
	///
	void invalid();

private:
	///
	boost::scoped_ptr<ButtonPolicy> bp_;
	///
	boost::scoped_ptr<BCView> view_;
};

#endif // BUTTONCONTROLLER_H
