// -*- C++ -*-
/* ButtonController.h
 * Controls the activation of the OK, Apply and Cancel buttons.
 * Actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 * Author: Allan Rae <rae@lyx.org>
 * This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include "ButtonPolicies.h"
#include <list>

#ifdef __GNUG__
#pragma interface
#endif

/** General purpose button controller for up to four buttons.
    Controls the activation of the OK, Apply and Cancel buttons.
    Actually supports 4 buttons in all and it's up to the user to decide on
    the activation policy and which buttons correspond to which output of the
    state machine.
    @author Allan Rae <rae@lyx.org>
    20001001 Switch from template implementation to taking Policy parameter.
             Allows FormBase to provide a ButtonController for any dialog.
*/
class ButtonController : public noncopyable
{
public:
	/** Constructor.
	    The cancel/close label entries are _not_ managed within the class
	    thereby allowing you to reassign at will and to use static labels.
	    It also means if you really don't want to have the Cancel button
	    label be different when there is nothing changed in the dialog then
	    you can just assign "Cancel" to both labels.  Or even reuse this
	    class for something completely different.
	 */
	ButtonController(ButtonPolicy * bp,
			 char const * cancel, char const * close);

	// Somebody else owns the FL_OBJECTs we just manipulate them.
	// so? (Lgb)
	//~ButtonController() {}

	/* Initialise Button Functions */
	/// Call refresh() when finished setting the buttons.
	void setOK(FL_OBJECT * obj) {
		okay_ = obj;
	}
	///
	void setApply(FL_OBJECT * obj) {
		apply_ = obj;
	}
	///
	void setCancel(FL_OBJECT * obj) {
		cancel_ = obj;
	}
	///
	void setUndoAll(FL_OBJECT * obj) {
		undo_all_ = obj;
	}
	///
	void setCancelTrueLabel(char const * c) {
		cancel_label = c;
	}
	///
	void setCancelFalseLabel(char const * c) {
		close_label = c;
	}
	///
	void addReadOnly(FL_OBJECT * obj) {
		read_only_.push_front(obj);
	}
	///
	void eraseReadOnly() {
		read_only_.erase(read_only_.begin(), read_only_.end());
	}

	/* Action Functions */
	/// force a refresh of the buttons
	void refresh();

	///
	void input(ButtonPolicy::SMInput in);
	///
	void ok();
	///
	void apply();
	///
	void cancel();
	///
	void undoAll();
	///
	void hide();
	/// Passthrough function -- returns its input value
	bool readOnly(bool ro = true);
	///
	void readWrite();
	/// Passthrough function -- returns its input value
	bool valid(bool v = true);
	///
	void invalid();
private:
	///
	ButtonPolicy * bp_;
	///
	FL_OBJECT * okay_;
	///
	FL_OBJECT * apply_;
	///
	FL_OBJECT * cancel_;
	///
	FL_OBJECT * undo_all_;
	/// List of items to be deactivated when in one of the read-only states
	std::list<FL_OBJECT *> read_only_;
	///
	char const * cancel_label;
	///
	char const * close_label;	
};

#endif
