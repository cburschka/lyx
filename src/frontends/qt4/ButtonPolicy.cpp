/**
 * \file ButtonPolicy.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ButtonPolicy.h"

#include "support/debug.h"

#include <iostream>
#include <vector>

using namespace std;

namespace lyx {
namespace frontend {

static char const * printState(ButtonPolicy::State const & state)
{
	switch (state) {
		case ButtonPolicy::INITIAL:
			return "INITIAL";
		case ButtonPolicy::VALID:
			return "VALID";
		case ButtonPolicy::INVALID:
			return "INVALID";
		case ButtonPolicy::APPLIED:
			return "APPLIED";
		case ButtonPolicy::AUTOAPPLY_INITIAL:
			return "AUTOAPPLY_INITIAL";
		case ButtonPolicy::AUTOAPPLY_CHANGED:
			return "AUTOAPPLY_CHANGED";
		case ButtonPolicy::RO_INITIAL:
			return "RO_INITIAL";
		case ButtonPolicy::RO_VALID:
			return "RO_VALID";
		case ButtonPolicy::RO_INVALID:
			return "RO_INVALID";
		case ButtonPolicy::RO_APPLIED:
			return "RO_APPLIED";
		case ButtonPolicy::RO_AUTOAPPLY:
			return "RO_AUTOAPPLY";
		case ButtonPolicy::BOGUS:
			return "BOGUS";
		default:
			return "";
	}
}


static char const * printInput(ButtonPolicy::SMInput const & input)
{
	switch (input) {
		case ButtonPolicy::SMI_VALID:
			return "SMI_VALID";
		case ButtonPolicy::SMI_INVALID:
			return "SMI_INVALID";
		case ButtonPolicy::SMI_OKAY:
			return "SMI_OKAY";
		case ButtonPolicy::SMI_APPLY:
			return "SMI_APPLY";
		case ButtonPolicy::SMI_CANCEL:
			return "SMI_CANCEL";
		case ButtonPolicy::SMI_RESTORE:
			return "SMI_RESTORE";
		case ButtonPolicy::SMI_AUTOAPPLY:
			return "SMI_AUTOAPPLY";
		case ButtonPolicy::SMI_HIDE:
			return "SMI_HIDE";
		case ButtonPolicy::SMI_READ_ONLY:
			return "SMI_READ_ONLY";
		case ButtonPolicy::SMI_READ_WRITE:
			return "SMI_READ_WRITE";
		case ButtonPolicy::SMI_NOOP:
			return "SMI_NOOP";
		case ButtonPolicy::SMI_TOTAL:
			return "SMI_TOTAL";
		default:
			return "";
	}
}


char const * functionName(ButtonPolicy::Policy policy)
{
	switch (policy) {
		case ButtonPolicy::PreferencesPolicy:
			return "PreferencesPolicy";
		case ButtonPolicy::OkCancelPolicy:
			return "OkCancelPolicy";
		case ButtonPolicy::OkCancelReadOnlyPolicy:
			return "OkCancelReadOnlyPolicy";
		case ButtonPolicy::OkApplyCancelPolicy:
			return "OkApplyCancelPolicy";
		case ButtonPolicy::OkApplyCancelReadOnlyPolicy:
			return "OkApplyCancelReadOnlyPolicy";
		case ButtonPolicy::OkApplyCancelAutoReadOnlyPolicy:
			return "OkApplyCancelAutoReadOnlyPolicy";
		case ButtonPolicy::NoRepeatedApplyPolicy:
			return "NoRepeatedApplyPolicy";
		case ButtonPolicy::NoRepeatedApplyReadOnlyPolicy:
			return "NoRepeatedApplyReadOnlyPolicy";
		case ButtonPolicy::IgnorantPolicy:
			return "IgnorantPolicy";
		default:
			return "Unknown policy";
	}
}


ostream & operator<<(ostream & os, ButtonPolicy::State st)
{
	return os << int(st);
}


ostream & operator<<(ostream & os, ButtonPolicy::SMInput smi)
{
	return os << int(smi);
}


/////////////////////////////////////////////////////////////////////////
//
// ButtonPolicy::Private
//
/////////////////////////////////////////////////////////////////////////

class ButtonPolicy::Private
{
public:
	typedef ButtonPolicy::SMInput SMInput;
	typedef ButtonPolicy::Policy Policy;
	typedef ButtonPolicy::State State;

	Private(Policy policy);

	void nextState(SMInput input);

	void initOkCancel();
	void initOkCancelReadOnly();
	void initNoRepeatedApplyReadOnly();
	void initOkApplyCancelReadOnly();
	void initOkApplyCancelAutoReadOnly();
	void initOkApplyCancel();
	void initNoRepeatedApply();
	void initPreferences();

public:
	///
	Policy policy_;

	/// Transition map of the state machine.
	typedef std::vector<State> StateArray;
	///
	typedef std::vector<StateArray> StateMachine;
	/// The state outputs are the status of the buttons.
	typedef std::vector<int> StateOutputs;

	/// Current state.
	State state_;
	/// Which buttons are active for a given state.
	StateOutputs outputs_;
	///
	StateMachine state_machine_;
};


ButtonPolicy::Private::Private(Policy policy)
{
	policy_ = policy;
	state_ = INITIAL;

	switch (policy_) {
		case OkCancelPolicy:
			initOkCancel();
			break;
		case OkCancelReadOnlyPolicy:
			initOkCancelReadOnly();
			break;
		case OkApplyCancelPolicy:
			initOkApplyCancel();
			break;
		case OkApplyCancelReadOnlyPolicy:
			initOkApplyCancelReadOnly();
			break;
		case OkApplyCancelAutoReadOnlyPolicy:
			initOkApplyCancelAutoReadOnly();
			break;
		case NoRepeatedApplyPolicy:
			initNoRepeatedApply();
			break;
		case NoRepeatedApplyReadOnlyPolicy:
			initNoRepeatedApplyReadOnly();
			break;
		case PreferencesPolicy:
			initPreferences();
			break;
		case IgnorantPolicy:
			break;
	}
}


void ButtonPolicy::Private::nextState(SMInput input)
{
	if (SMI_NOOP == input)
		return;

	State tmp = state_machine_[state_][input];

	LYXERR(Debug::GUI, "Transition from state "
			   << printState(state_) << " to state "
			   << printState(tmp) << " after input "
			   << printInput(input));

	if (tmp != BOGUS) {
		state_ = tmp;
	} else {
		LYXERR0(functionName(policy_) << ": No transition for input "
	    << printInput(input) << " from state " << printState(state_));
	}
}


void ButtonPolicy::Private::initPreferences()
{
	outputs_ = StateOutputs(APPLIED + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(APPLIED + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | APPLY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;
	outputs_[APPLIED] = OKAY | CLOSE;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input().  This won't necessarily be true for all
	//        policies though so I'll leave those two as distinct
	//        inputs rather than merge them.  For example, a dialog
	//        that doesn't update it's input fields when reshown
	//        after being hidden needs a policy where CANCEL and
	//        HIDE are treated differently.
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_ONLY] = INITIAL;
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_ONLY] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_APPLY] = APPLIED;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	// State::INVALID
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
	// State::APPLIED
	state_machine_[APPLIED][SMI_VALID] = VALID;
	state_machine_[APPLIED][SMI_INVALID] = INVALID;
	state_machine_[APPLIED][SMI_OKAY] = INITIAL;
	state_machine_[APPLIED][SMI_READ_ONLY] = APPLIED;
	state_machine_[APPLIED][SMI_READ_WRITE] = APPLIED;
}


void ButtonPolicy::Private::initOkCancel()
{
	outputs_ = StateOutputs(INVALID + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_ONLY] = INITIAL;
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_ONLY] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	// State::INVALID
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
}


void ButtonPolicy::Private::initOkCancelReadOnly()
{
	outputs_ = StateOutputs(RO_INVALID + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(RO_INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = RESTORE | CANCEL;
	outputs_[RO_INVALID] = RESTORE | CANCEL;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	state_machine_[INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
	state_machine_[INVALID][SMI_READ_ONLY] = RO_INVALID;
	// State::RO_INITIAL
	state_machine_[RO_INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	state_machine_[RO_INITIAL][SMI_VALID] = RO_VALID;
	state_machine_[RO_INITIAL][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INITIAL][SMI_READ_WRITE] = INITIAL;
	// State::RO_VALID
	state_machine_[RO_VALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_VALID][SMI_READ_ONLY] = RO_VALID;
	state_machine_[RO_VALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_VALID][SMI_READ_WRITE] = VALID;
	state_machine_[RO_VALID][SMI_RESTORE] = RO_INITIAL;
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[RO_INVALID][SMI_RESTORE] = RO_INITIAL;
}


void ButtonPolicy::Private::initNoRepeatedApplyReadOnly()
{
	outputs_ = StateOutputs(RO_INVALID + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(RO_INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | APPLY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = RESTORE | CANCEL;
	outputs_[RO_INVALID] = RESTORE | CANCEL;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	state_machine_[INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_APPLY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_OKAY] = INITIAL;
	state_machine_[INVALID][SMI_APPLY] = INITIAL;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
	state_machine_[INVALID][SMI_READ_ONLY] = RO_INVALID;
	// State::RO_INITIAL
	state_machine_[RO_INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	state_machine_[RO_INITIAL][SMI_VALID] = RO_VALID;
	state_machine_[RO_INITIAL][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INITIAL][SMI_READ_WRITE] = INITIAL;
	// State::RO_VALID
	state_machine_[RO_VALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_VALID][SMI_READ_ONLY] = RO_VALID;
	state_machine_[RO_VALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_VALID][SMI_READ_WRITE] = VALID;
	state_machine_[RO_VALID][SMI_RESTORE] = RO_INITIAL;
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[RO_INVALID][SMI_RESTORE] = RO_INITIAL;
}


void ButtonPolicy::Private::initOkApplyCancelReadOnly()
{
	outputs_ = StateOutputs(RO_APPLIED + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(RO_APPLIED + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | APPLY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;
	outputs_[APPLIED] = OKAY | APPLY | CLOSE;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = RESTORE | CANCEL;
	outputs_[RO_INVALID] = RESTORE | CANCEL;
	outputs_[RO_APPLIED] = CLOSE;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	state_machine_[INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	state_machine_[VALID][SMI_APPLY] = APPLIED;
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
	state_machine_[INVALID][SMI_READ_ONLY] = RO_INVALID;
	// State::APPLIED
	state_machine_[APPLIED][SMI_APPLY] = APPLIED;
	state_machine_[APPLIED][SMI_READ_WRITE] = APPLIED;
	state_machine_[APPLIED][SMI_VALID] = VALID;
	state_machine_[APPLIED][SMI_INVALID] = INVALID;
	state_machine_[APPLIED][SMI_OKAY] = INITIAL;
	state_machine_[APPLIED][SMI_READ_ONLY] = RO_APPLIED;
	// State::RO_INITIAL
	state_machine_[RO_INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	state_machine_[RO_INITIAL][SMI_VALID] = RO_VALID;
	state_machine_[RO_INITIAL][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INITIAL][SMI_READ_WRITE] = INITIAL;
	// State::RO_VALID
	state_machine_[RO_VALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_VALID][SMI_READ_ONLY] = RO_VALID;
	state_machine_[RO_VALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_VALID][SMI_READ_WRITE] = VALID;
	state_machine_[RO_VALID][SMI_RESTORE] = RO_INITIAL;
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[RO_INVALID][SMI_RESTORE] = RO_INITIAL;
	// State::RO_APPLIED
	state_machine_[RO_APPLIED][SMI_READ_ONLY] = RO_APPLIED;
	state_machine_[RO_APPLIED][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_APPLIED][SMI_VALID] = RO_VALID;
	state_machine_[RO_APPLIED][SMI_READ_WRITE] = APPLIED;
}


void ButtonPolicy::Private::initOkApplyCancel()
{
	outputs_ = StateOutputs(APPLIED + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(APPLIED + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | APPLY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;
	outputs_[APPLIED] = OKAY | APPLY | CLOSE;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_ONLY] = INITIAL;
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_ONLY] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	state_machine_[VALID][SMI_APPLY] = APPLIED;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
	// State::APPLIED
	state_machine_[APPLIED][SMI_APPLY] = APPLIED;
	state_machine_[APPLIED][SMI_READ_ONLY] = APPLIED;
	state_machine_[APPLIED][SMI_READ_WRITE] = APPLIED;
	state_machine_[APPLIED][SMI_VALID] = VALID;
	state_machine_[APPLIED][SMI_INVALID] = INVALID;
	state_machine_[APPLIED][SMI_OKAY] = INITIAL;
}


void ButtonPolicy::Private::initOkApplyCancelAutoReadOnly()
{
	outputs_ = StateOutputs(RO_AUTOAPPLY + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(RO_AUTOAPPLY + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE | AUTOAPPLY;
	outputs_[VALID] = RESTORE | OKAY | APPLY | CANCEL | AUTOAPPLY;
	outputs_[INVALID] = RESTORE | CANCEL | AUTOAPPLY;
	outputs_[APPLIED] = OKAY | CLOSE | AUTOAPPLY;
	outputs_[AUTOAPPLY_INITIAL] = CLOSE | AUTOAPPLY | OKAY;
	outputs_[AUTOAPPLY_CHANGED] = CLOSE | RESTORE | AUTOAPPLY | OKAY;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = RESTORE | CANCEL;
	outputs_[RO_INVALID] = RESTORE | CANCEL;
	outputs_[RO_APPLIED] = CLOSE;
	outputs_[RO_AUTOAPPLY] = CLOSE;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	state_machine_[INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	state_machine_[INITIAL][SMI_AUTOAPPLY] = AUTOAPPLY_INITIAL;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	state_machine_[VALID][SMI_APPLY] = APPLIED;
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	state_machine_[VALID][SMI_AUTOAPPLY] = AUTOAPPLY_INITIAL;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
	state_machine_[INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[INVALID][SMI_AUTOAPPLY] = AUTOAPPLY_CHANGED;
	// State::APPLIED
	state_machine_[APPLIED][SMI_APPLY] = APPLIED;
	state_machine_[APPLIED][SMI_READ_WRITE] = APPLIED;
	state_machine_[APPLIED][SMI_VALID] = VALID;
	state_machine_[APPLIED][SMI_INVALID] = INVALID;
	state_machine_[APPLIED][SMI_OKAY] = INITIAL;
	state_machine_[APPLIED][SMI_READ_ONLY] = RO_APPLIED;
	state_machine_[APPLIED][SMI_AUTOAPPLY] = AUTOAPPLY_INITIAL;
	// State::AUTOAPPLY_INITIAL
	state_machine_[AUTOAPPLY_INITIAL][SMI_AUTOAPPLY] = APPLIED;
	state_machine_[AUTOAPPLY_INITIAL][SMI_READ_ONLY] = RO_AUTOAPPLY;
	state_machine_[AUTOAPPLY_INITIAL][SMI_VALID] = AUTOAPPLY_CHANGED;
	state_machine_[AUTOAPPLY_INITIAL][SMI_INVALID] = AUTOAPPLY_CHANGED;	
	state_machine_[AUTOAPPLY_INITIAL][SMI_READ_WRITE] = AUTOAPPLY_INITIAL;	
	// State::AUTOAPPLY_CHANGED
	state_machine_[AUTOAPPLY_CHANGED][SMI_AUTOAPPLY] = APPLIED;
	state_machine_[AUTOAPPLY_CHANGED][SMI_READ_ONLY] = RO_AUTOAPPLY;
	state_machine_[AUTOAPPLY_CHANGED][SMI_RESTORE] = AUTOAPPLY_INITIAL;
	state_machine_[AUTOAPPLY_CHANGED][SMI_VALID] = AUTOAPPLY_CHANGED;
	state_machine_[AUTOAPPLY_CHANGED][SMI_INVALID] = AUTOAPPLY_CHANGED;
	state_machine_[AUTOAPPLY_CHANGED][SMI_READ_WRITE] = AUTOAPPLY_CHANGED;
	state_machine_[AUTOAPPLY_CHANGED][SMI_APPLY] = AUTOAPPLY_INITIAL;
	// State::RO_INITIAL
	state_machine_[RO_INITIAL][SMI_READ_ONLY] = RO_INITIAL;
	state_machine_[RO_INITIAL][SMI_VALID] = RO_VALID;
	state_machine_[RO_INITIAL][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[RO_INITIAL][SMI_AUTOAPPLY] = RO_AUTOAPPLY;
	// State::RO_VALID
	state_machine_[RO_VALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_VALID][SMI_READ_ONLY] = RO_VALID;
	state_machine_[RO_VALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_VALID][SMI_READ_WRITE] = VALID;
	state_machine_[RO_VALID][SMI_RESTORE] = RO_INITIAL;
	state_machine_[RO_VALID][SMI_AUTOAPPLY] = RO_AUTOAPPLY;
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[RO_INVALID][SMI_RESTORE] = RO_INITIAL;
	state_machine_[RO_INVALID][SMI_AUTOAPPLY] = RO_AUTOAPPLY;
	// State::RO_APPLIED
	state_machine_[RO_APPLIED][SMI_READ_ONLY] = RO_APPLIED;
	state_machine_[RO_APPLIED][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_APPLIED][SMI_VALID] = RO_VALID;
	state_machine_[RO_APPLIED][SMI_READ_WRITE] = APPLIED;
	state_machine_[RO_APPLIED][SMI_AUTOAPPLY] = RO_AUTOAPPLY;
	// State::RO_AUTOAPPLY
	state_machine_[RO_AUTOAPPLY][SMI_READ_WRITE] = AUTOAPPLY_INITIAL;
}


void ButtonPolicy::Private::initNoRepeatedApply()
{
	outputs_ = StateOutputs(INVALID + 1, ButtonPolicy::ALL_BUTTONS);
	state_machine_ = StateMachine(INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS));

	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = RESTORE | OKAY | APPLY | CANCEL;
	outputs_[INVALID] = RESTORE | CANCEL;

	// Build the state machine one state at a time
	// NOTE:  Since CANCEL and HIDE always go to INITIAL they are
	//        left out of the state machine and handled explicitly
	//        in input()
	//
	// State::INITIAL
	state_machine_[INITIAL][SMI_READ_ONLY] = INITIAL;
	state_machine_[INITIAL][SMI_READ_WRITE] = INITIAL;
	state_machine_[INITIAL][SMI_VALID] = VALID;
	state_machine_[INITIAL][SMI_INVALID] = INVALID;
	// State::VALID
	state_machine_[VALID][SMI_VALID] = VALID;
	state_machine_[VALID][SMI_READ_ONLY] = VALID;
	state_machine_[VALID][SMI_READ_WRITE] = VALID;
	state_machine_[VALID][SMI_INVALID] = INVALID;
	state_machine_[VALID][SMI_OKAY] = INITIAL;
	state_machine_[VALID][SMI_APPLY] = INITIAL;
	state_machine_[VALID][SMI_RESTORE] = INITIAL;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_RESTORE] = INITIAL;
}


/////////////////////////////////////////////////////////////////////////
//
// ButtonPolicy
//
/////////////////////////////////////////////////////////////////////////

ButtonPolicy::ButtonPolicy(Policy policy)
	: d(new Private(policy))
{}


ButtonPolicy::~ButtonPolicy()
{
	delete d;
}


void ButtonPolicy::setPolicy(Policy policy)
{
	*d = Private(policy);
}


void ButtonPolicy::input(SMInput input)
{
	switch (d->policy_) {
		case PreferencesPolicy:
			// The APPLIED state is persistent. Next time the dialog is opened,
			// the user will be able to press 'Save'.
			if (SMI_CANCEL == input || SMI_HIDE == input) {
				if (d->state_ != APPLIED)
					d->state_ = INITIAL;
			} else {
				d->nextState(input);
			}
			break;
		case IgnorantPolicy:
			break;
		default:
			// CANCEL and HIDE always take us to INITIAL for all cases
			if (SMI_CANCEL == input || SMI_HIDE == input) {
				if (d->state_ == AUTOAPPLY_INITIAL
					  || d->state_ == AUTOAPPLY_CHANGED)
					d->state_ = AUTOAPPLY_INITIAL;
				else
					d->state_ = INITIAL;
			} else
				d->nextState(input);
			break;
	}
}


bool ButtonPolicy::buttonStatus(Button button) const
{
	return d->policy_ == IgnorantPolicy || (button & d->outputs_[d->state_]);
}


bool ButtonPolicy::isReadOnly() const
{
	switch (d->policy_) {
		case NoRepeatedApplyReadOnlyPolicy:
		case OkCancelReadOnlyPolicy:
		case OkApplyCancelReadOnlyPolicy:
			return RO_INITIAL == d->state_
				|| RO_VALID == d->state_
				|| RO_INVALID == d->state_
				|| RO_APPLIED == d->state_;
		default:
			return false;
	}
}



} // namespace frontend
} // namespace lyx
