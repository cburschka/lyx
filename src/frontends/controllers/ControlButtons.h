/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlButtonss.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
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

#include "DialogBase.h" // This can go eventually
#include "ButtonControllerBase.h"

class ViewBase;

/** Abstract base class for Controllers with a ButtonController.
 */
class ControlButtons : public DialogBase
{
public: // methods
	///
	ControlButtons() {}
	///
	virtual ~ControlButtons() {};

	/// These functions are called when the controlling buttons are pressed.
	///
	void ApplyButton();
	///
	void OKButton();
	///
	void CancelButton();
	///
	void RestoreButton();

	/** Allow the view to access the ButtonController. This method must be
	    instantiated in a daughter class that creates the actual instance
	    of the ButtonController. */
	virtual ButtonControllerBase & bc() = 0;

protected:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply() = 0;
	/// Disconnect signals and hide View.
	virtual void hide() = 0;
	/// Update dialog before showing it.
	virtual void update() = 0;

	/** Allow the Controller to access the View. This method must be
	    instantiated in a daughter class that creates the actual instance
	    of the View. */
	virtual ViewBase & view() = 0;
};

#include "ViewBase.h"

#endif // CONTROLBUTTONS_H
