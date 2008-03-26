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

using namespace std;

namespace lyx {
namespace frontend {


ButtonPolicy::ButtonPolicy(Policy policy)
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


void ButtonPolicy::input(SMInput input)
{
	switch (policy_) {
		case PreferencesPolicy:
			// The APPLIED state is persistent. Next time the dialog is opened,
			// the user will be able to press 'Save'.
			if (SMI_CANCEL == input || SMI_HIDE == input) {
				if (state_ != APPLIED)
					state_ = INITIAL;
			} else {
				nextState(input);
			}
			break;
		case IgnorantPolicy:
			break;
		default:
			// CANCEL and HIDE always take us to INITIAL for all cases
			if (SMI_CANCEL == input || SMI_HIDE == input)
				state_ = INITIAL;
			else
				nextState(input);
			break;
	}
}


bool ButtonPolicy::buttonStatus(Button button) const
{
	return policy_ == IgnorantPolicy ? true : button & outputs_[state_];
}


bool ButtonPolicy::isReadOnly() const
{
	switch(policy_) {
		case NoRepeatedApplyReadOnlyPolicy:
		case OkCancelReadOnlyPolicy:
		case OkApplyCancelReadOnlyPolicy:
			return RO_INITIAL == state_
				|| RO_VALID == state_
				|| RO_INVALID == state_
				|| RO_APPLIED == state_;
		default:
			return false;
	}
}


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
		case ButtonPolicy::RO_INITIAL:
			return "RO_INITIAL";
		case ButtonPolicy::RO_VALID:
			return "RO_VALID";
		case ButtonPolicy::RO_INVALID:
			return "RO_INVALID";
		case ButtonPolicy::RO_APPLIED:
			return "RO_APPLIED";
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


void ButtonPolicy::nextState(SMInput input)
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
		lyxerr << functionName(policy_)
		       << ": No transition for input "
		       << printInput(input)
		       << " from state "
		       << printState(state_)
		       << endl;
	}
}


void ButtonPolicy::initPreferences()
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


void ButtonPolicy::initOkCancel()
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


void ButtonPolicy::initOkCancelReadOnly()
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


void ButtonPolicy::initNoRepeatedApplyReadOnly()
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
	state_machine_[INVALID][SMI_OKAY] = INVALID;
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
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[RO_INVALID][SMI_RESTORE] = RO_INITIAL;
}


void ButtonPolicy::initOkApplyCancelReadOnly()
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


void ButtonPolicy::initOkApplyCancel()
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


void ButtonPolicy::initNoRepeatedApply()
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


ostream & operator<<(ostream & os, ButtonPolicy::State st)
{
	return os << int(st);
}


ostream & operator<<(ostream & os, ButtonPolicy::SMInput smi)
{
	return os << int(smi);
}


} // namespace frontend
} // namespace lyx
