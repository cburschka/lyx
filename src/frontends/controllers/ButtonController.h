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


class BCView;


/** \class ButtonController controls the activation of the OK, Apply and
 *  Cancel buttons.
 *
 * It actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 */

class ButtonController : boost::noncopyable {
public:
	//@{
	/** Methods to set and get the GUI view (containing the actual
	 *   button widgets.
	 *  \param ptr is owned by the ButtonController.
	 */
	void view(BCView * ptr);
	BCView & view() const;
	//@}

	//@{
	/** Methods to set and get the ButtonPolicy.
	 *  \param ptr is owned by the ButtonController.
	 */
	void bp(ButtonPolicy * ptr);
	ButtonPolicy & bp() const;
	//@}

	///
	void input(ButtonPolicy::SMInput) const;

	//@{
	/// Tell the BC that a particular button has been pressed.
	void ok() const;
	void apply() const;
	void cancel() const;
	void restore() const;
	//@}

	/// Tell the BC that the dialog is being hidden
	void hide() const;

	/**Refresh the activation state of the Ok, Apply, Close and
	 * Restore buttons.
	 */
	void refresh() const;

	/** Refresh the activation state of all the widgets under the control
	 *  of the BC to reflect the read-only status of the underlying buffer.
	 */
	void refreshReadOnly() const;
	//@}

	/** Passthrough function -- returns its input value
	 *  Tell the BC about the read-only status of the underlying buffer.
	 */
	bool readOnly(bool = true) const;

	/** \param validity Tell the BC that the data is, or is not, valid.
	 *  Sets the activation state of the buttons immediately.
	 */
	void valid(bool = true) const;

private:
	boost::scoped_ptr<ButtonPolicy> bp_;
	boost::scoped_ptr<BCView> view_;
};

#endif // BUTTONCONTROLLER_H
