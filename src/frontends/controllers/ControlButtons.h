// -*- C++ -*-
/**
 * \file ControlButtons.h
 * Read the file COPYING
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * ControlButtons serves only to control the activation of the Ok, Apply, Cancel
 * and Restore buttons on the View dialog.
 *
 * More generally, the class is part of a hierarchy of controller classes
 * that together connect the GUI-dependent dialog to any appropriate
 * signals and dispatches any changes to the kernel.
 *
 * These controllers have no knowledge of the actual instantiation of the
 * GUI-dependent View and ButtonController, which should therefore
 * be created elsewhere.
 *
 * Once created, the Controller will take care of their initialisation,
 * management and, ultimately, destruction.
 */

#ifndef CONTROLBUTTONS_H
#define CONTROLBUTTONS_H

#ifdef __GNUG__
#pragma interface
#endif

#include <boost/utility.hpp>

class ViewBase;
class ButtonControllerBase;

/** Abstract base class for Controllers with a ButtonController.
 */
class ControlButtons : boost::noncopyable
{
public:
	///
	ControlButtons();
	///
	virtual ~ControlButtons() {}

	/** These functions are called by the view when the appropriate buttons
	 *  are pressed.
	 */
	///
	void ApplyButton();
	///
	void OKButton();
	///
	void CancelButton();
	///
	void RestoreButton();

	/// Returns the user-specified iconification policy.
	bool IconifyWithMain() const;

	///
	ButtonControllerBase & bc();

	///
	void setView(ViewBase &);
	///
	void setButtonController(ButtonControllerBase &);

protected:
	///
	ViewBase & view();

	/** When Applying it's useful to know whether the dialog is about
	    to close or not (no point refreshing the display for example). */
	bool isClosing() const { return is_closing_; }

	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply() = 0;
	/// Disconnect signals and hide View.
	virtual void hide() = 0;
	/// Update dialog before showing it.
	virtual void update() = 0;

	/** This flag can be set by one of the miriad the controller methods
	    to ensure that the dialog is shut down. */
	bool emergency_exit_;

private:
	///
	bool is_closing_;
	/// We own neither of these pointers.
	ButtonControllerBase * bc_ptr_;
	///
	ViewBase * view_ptr_;
};

#endif // CONTROLBUTTONS_H
