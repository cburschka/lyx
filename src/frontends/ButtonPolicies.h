// -*- C++ -*-
/* ButtonPolicies.h
 * Provides a state machine implementation of the various button policies
 * used by the dialogs.
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

#ifndef BUTTONPOLICIES_H
#define BUTTONPOLICIES_H


#include <vector>
#include "support/utility.hpp"


/** An abstract base class for button policies.
    A state machine implementation of the various button policies used by the
    dialogs. Only the policy is implemented here.  Separate ButtonController
    classes are needed for each GUI implementation.

		Policy		| ReadOnly | Apply Button | Repeated Apply
    ========================================================================
    OkCancel			|	N  |	N	  |	-
    OkCancelReadOnly		|	Y  |	N	  |	-
    OkApplyCancel		|	N  |	Y	  |	Y
    OkApplyCancelReadOnly	|	Y  |	Y	  |	Y
    NoRepeatedApply		|	N  |	Y	  |	N
    NoRepeatedApplyReadOnly	|	Y  |	Y	  |	N
    Preferences			|	N  |	Y	  | No (Ok-Close)
    ========================================================================

    Policy
	The name of the policy
    ReadOnly
	Does the policy treat read-only docs differently to read-write docs?
	This usually means that when an SMI_READ_ONLY input arrives then
	all the buttons are disabled except Cancel/Close.  The state
	machine tracks the inputs (valid/invalid) and has states for all
	combinations. When an SMI_READ_WRITE input arrives the appropriate
	machine state is entered (just as if the document had always been
	read-write).
	NOTE: If a dialog doesn't care about the read-only status of a document
	(and uses an appropriate policy) it can never get into a read-only state
	so isReadOnly() can only ever return false even though the document may
	be read-only.
    Repeated Apply
	Simply means that it is alright to use the Apply button multiple times
	without requiring a change of the dialog contents.  If no repeating is
	allowed the Ok+Apply buttons are deactivated.  The Preferences dialog
	has its own special version of repeated apply handling because its Ok
	button is actually a Save button -- its always reasonable to Save the
	preferences if the dialog has changed since the last save.
 */
class ButtonPolicy : public noncopyable {
public:
	///
	virtual ~ButtonPolicy() {}

	/** The various possible state names.
	    Not all state-machines have this many states.  However, we need
	    to define them all here so we can share the code.
	*/
	enum State {
		///
		INITIAL = 0,
		///
		VALID,
		///
		INVALID,
		///
		APPLIED,
		///
		RO_INITIAL,
		///
		RO_VALID,
		///
		RO_INVALID,
		///
		RO_APPLIED,
		///
		BOGUS = 55
	};
	
	/// The various button types.
	enum Button {
		///
		CLOSE    = 0,  // Not a real button, but effectively !CANCEL
		///
		OKAY     = 1,
		///
		APPLY    = 2,
		///
		CANCEL   = 4,
		///
		UNDO_ALL = 8
	};
	///
	static const Button ALL_BUTTONS =
		Button(OKAY | APPLY | CANCEL | UNDO_ALL);
  
	/** State machine inputs.
	    All the policies so far have both CANCEL and HIDE always going to
	    INITIAL. This won't necessarily be true for all [future] policies
	    though so I'll leave those two as distinct inputs rather than merge
	    them.  For example, a dialog that doesn't update it's input fields
	    when reshown after being hidden needs a policy where CANCEL and
	    HIDE are treated differently.
	 */
	enum SMInput {
		///
		SMI_VALID = 0,
		///
		SMI_INVALID,
		///
		SMI_OKAY,
		///
		SMI_APPLY,
		///
		SMI_CANCEL,
		///
		SMI_UNDO_ALL,
		///
		SMI_HIDE,
		///
		SMI_READ_ONLY,
		///
		SMI_READ_WRITE,
		///
		SMI_TOTAL	// not a real input
	};

	/// Trigger a transition with this input.
	virtual void input(SMInput) = 0;
	/// Activation status of a button
	virtual bool buttonStatus(Button) const = 0;
	/// Are we in a read-only state?
	virtual bool isReadOnly() const = 0;

	/// Transition map of the state machine.
	typedef std::vector<State> StateArray;
	///
	typedef std::vector<StateArray> StateMachine;
	/// The state outputs are the status of the buttons.
	typedef std::vector<int> StateOutputs;
};


//--------------------- Actual Policy Classes -----------------------------

/** Ok and Cancel buttons for dialogs with read-only operation.
    Note: This scheme supports the relabelling of Cancel to Close and
    vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class OkCancelPolicy : public ButtonPolicy {
public:
	///
	OkCancelPolicy();
	///
	virtual ~OkCancelPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/** Activation status of a button.
	    We assume that we haven't gotten into an undefined state.
	    This is reasonable since we can only reach states defined
	    in the state machine and they should all have been defined in
	    the outputs_ variable.  Perhaps we can do something at compile
	    time to check that all the states have corresponding outputs.
	 */
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return false;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


/** Ok and Cancel buttons for dialogs where read-only operation is blocked.
    The state machine design for this policy allows changes to occur within
    the dialog while a file is read-only -- the okay button is disabled until
    a read-write input is given.  When the file is made read-write the dialog
    will then be in the correct state (as if the file had always been read-write).
    Note: This scheme supports the relabelling of Cancel to Close and vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class OkCancelReadOnlyPolicy : public ButtonPolicy {
public:
	///
	OkCancelReadOnlyPolicy();
	///
	virtual ~OkCancelReadOnlyPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/// Activation status of a button.
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return RO_INITIAL == state_
			|| RO_VALID == state_
			|| RO_INVALID == state_
			|| RO_APPLIED == state_;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


/** Ok, Apply and Cancel buttons for dialogs where read-only operation is blocked.
    Repeated Apply are not allowed.  Likewise,  Ok cannot follow Apply without
    some valid input. That is, the dialog contents must change between each Apply
    or Apply and Ok.
    The state machine design for this policy allows changes to occur within
    the dialog while a file is read-only -- the Ok+Apply buttons are disabled
    until a read-write input is given.  When the file is made read-write the
    dialog will then be in the correct state (as if the file had always been
    read-write).
    Note: This scheme supports the relabelling of Cancel to Close and vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class NoRepeatedApplyReadOnlyPolicy : public ButtonPolicy
{
public:
	///
	NoRepeatedApplyReadOnlyPolicy();
	///
	virtual ~NoRepeatedApplyReadOnlyPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/// Activation status of a button.
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return RO_INITIAL == state_
			|| RO_VALID == state_
			|| RO_INVALID == state_
			|| RO_APPLIED == state_;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


/** Ok, Apply and Cancel buttons for dialogs where read-only operation is blocked.
    Repeated Apply is allowed.  Likewise,  Ok can follow Apply.
    The state machine design for this policy allows changes to occur within
    the dialog while a file is read-only -- the Ok+Apply buttons are disabled
    until a read-write input is given.  When the file is made read-write the
    dialog will then be in the correct state (as if the file had always been
    read-write).
    Note: This scheme supports the relabelling of Cancel to Close and vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class OkApplyCancelReadOnlyPolicy : public ButtonPolicy {
public:
	///
	OkApplyCancelReadOnlyPolicy();
	///
	virtual ~OkApplyCancelReadOnlyPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/// Activation status of a button.
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return RO_INITIAL == state_
			|| RO_VALID == state_
			|| RO_INVALID == state_
			|| RO_APPLIED == state_;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


/** Ok, Apply and Cancel buttons for dialogs where repeated Apply is allowed.
    Note: This scheme supports the relabelling of Cancel to Close and vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class OkApplyCancelPolicy : public ButtonPolicy {
public:
	///
	OkApplyCancelPolicy();
	///
	virtual ~OkApplyCancelPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/// Activation status of a button.
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return false;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


/** Ok, Apply and Cancel buttons for dialogs with no repeated Apply.
    Note: This scheme supports the relabelling of Cancel to Close and vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class NoRepeatedApplyPolicy : public ButtonPolicy {
public:
	///
	NoRepeatedApplyPolicy();
	///
	virtual ~NoRepeatedApplyPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/// Activation status of a button.
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return false;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


/** Defines the policy used by the Preferences dialog.
    Four buttons: Ok (Save), Apply, Cancel/Close, Restore.
    Note: This scheme supports the relabelling of Cancel to Close and vice versa.
    This is based on the value of the bool state of the Button::CANCEL.
    true == Cancel, false == Close
 */
class PreferencesPolicy : public ButtonPolicy {
public:
	///
	PreferencesPolicy();
	///
	virtual ~PreferencesPolicy() {}
	
	/// Trigger a transition with this input.
	virtual void input(SMInput);
	/// Activation status of a button.
	virtual bool buttonStatus(Button button) const {
		return button & outputs_[state_];
	}
	/// Are we in a read-only state?
	virtual bool isReadOnly() const {
		return false;
	}
private:
	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};

#endif
