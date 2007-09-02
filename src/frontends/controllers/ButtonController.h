// -*- C++ -*-
/**
 * \file ButtonController.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include "ButtonPolicy.h"
#include "BCView.h"

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace lyx {
namespace frontend {

/** \c ButtonController controls the activation of the OK, Apply and
 *  Cancel buttons.
 *
 * It actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 */

class ButtonController : boost::noncopyable {
public:
	ButtonController() : policy_(ButtonPolicy::IgnorantPolicy) {}
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
	void setPolicy(ButtonPolicy::Policy policy);
	ButtonPolicy const & policy() const { return policy_; }
	ButtonPolicy & policy() { return policy_; }
	//@}

	///
	void input(ButtonPolicy::SMInput);

	//@{
	/// Tell the BC that a particular button has been pressed.
	void ok();
	void apply();
	void cancel();
	void restore();
	//@}

	/// Tell the BC that the dialog is being hidden
	void hide();

	/**Refresh the activation state of the Ok, Apply, Close and
	 * Restore buttons.
	 */
	void refresh() const;

	/** Refresh the activation state of all the widgets under the control
	 *  of the BC to reflect the read-only status of the underlying buffer.
	 */
	void refreshReadOnly() const;

	/** Passthrough function -- returns its input value
	 *  Tell the BC about the read-only status of the underlying buffer.
	 */
	bool readOnly(bool = true);

	/** \param validity Tell the BC that the data is, or is not, valid.
	 *  Sets the activation state of the buttons immediately.
	 */
	void valid(bool = true);

private:
	ButtonPolicy policy_;
	boost::scoped_ptr<BCView> view_;
};

} // namespace frontend
} // namespace lyx

#endif // BUTTONCONTROLLER_H
