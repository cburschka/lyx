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
#include "debug.h"
#include <string>

using std::endl;
using std::string;

namespace lyx {
namespace frontend {

namespace {

string const printState(ButtonPolicy::State const & state)
{
	string output;

	switch(state) {
	case ButtonPolicy::INITIAL:
		output = "INITIAL";
		break;
	case ButtonPolicy::VALID:
		output = "VALID";
		break;
	case ButtonPolicy::INVALID:
		output = "INVALID";
		break;
	case ButtonPolicy::APPLIED:
		output = "APPLIED";
		break;
	case ButtonPolicy::RO_INITIAL:
		output = "RO_INITIAL";
		break;
	case ButtonPolicy::RO_VALID:
		output = "RO_VALID";
		break;
	case ButtonPolicy::RO_INVALID:
		output = "RO_INVALID";
		break;
	case ButtonPolicy::RO_APPLIED:
		output = "RO_APPLIED";
		break;
	case ButtonPolicy::BOGUS:
		output = "BOGUS";
		break;
	}

	return output;
}


string const printInput(ButtonPolicy::SMInput const & input)
{
	string output;

	switch (input) {
	case ButtonPolicy::SMI_VALID:
		output = "SMI_VALID";
		break;
	case ButtonPolicy::SMI_INVALID:
		output = "SMI_INVALID";
		break;
	case ButtonPolicy::SMI_OKAY:
		output = "SMI_OKAY";
		break;
	case ButtonPolicy::SMI_APPLY:
		output = "SMI_APPLY";
		break;
	case ButtonPolicy::SMI_CANCEL:
		output = "SMI_CANCEL";
		break;
	case ButtonPolicy::SMI_RESTORE:
		output = "SMI_RESTORE";
		break;
	case ButtonPolicy::SMI_HIDE:
		output = "SMI_HIDE";
		break;
	case ButtonPolicy::SMI_READ_ONLY:
		output = "SMI_READ_ONLY";
		break;
	case ButtonPolicy::SMI_READ_WRITE:
		output = "SMI_READ_WRITE";
		break;
	case ButtonPolicy::SMI_NOOP:
		output = "SMI_NOOP";
		break;
	case ButtonPolicy::SMI_TOTAL:
		output = "SMI_TOTAL";
		break;
	}

	return output;
}


/// Helper function
void nextState(ButtonPolicy::State & state,
	       ButtonPolicy::SMInput in,
	       ButtonPolicy::StateMachine const & s_m,
	       char const * function_name = "nextState")
{
	if (ButtonPolicy::SMI_NOOP == in)
	return;

	ButtonPolicy::State tmp = s_m[state][in];

	LYXERR(Debug::GUI) << "Transition from state "
			   << printState(state) << " to state "
			   << printState(tmp) << " after input "
			   << printInput(in) << std::endl;

	if (ButtonPolicy::BOGUS != tmp) {
		state = tmp;
	} else {
		lyxerr << function_name
		       << ": No transition for input "
		       << printInput(in)
		       << " from state "
		       << printState(state)
		       << endl;
	}
}

} // namespace anon


/*-----------------------------PreferencesPolicy-----------------------------*/


PreferencesPolicy::PreferencesPolicy()
	: state_(INITIAL),
	  outputs_(APPLIED + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(APPLIED + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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


void PreferencesPolicy::input(SMInput input)
{
	// The APPLIED state is persistent. Next time the dialog is opened,
	// the user will be able to press 'Save'.
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		if (state_ != APPLIED)
			state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_, "PreferencesPolicy");
	}
}


/*-------------------------------OkCancelPolicy------------------------------*/


OkCancelPolicy::OkCancelPolicy()
	: state_(INITIAL),
	  outputs_(INVALID + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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



void OkCancelPolicy::input(SMInput input)
{
	//lyxerr << "OkCancelPolicy::input" << endl;

	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_, "OkCancelPolicy");
	}
}


/*---------------------------OkCancelReadOnlyPolicy-------------------------*/


OkCancelReadOnlyPolicy::OkCancelReadOnlyPolicy()
	: state_(INITIAL),
	  outputs_(RO_INVALID + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(RO_INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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


void OkCancelReadOnlyPolicy::input(SMInput input)
{
	//lyxerr << "OkCancelReadOnlyPolicy::input" << endl;

	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_,
			  "OkCancelReadOnlyPolicy");
	}
}


/*--------------------------NoRepeatedApplyReadOnlyPolicy----------------------*/


NoRepeatedApplyReadOnlyPolicy::NoRepeatedApplyReadOnlyPolicy()
	: state_(INITIAL),
	  outputs_(RO_INVALID + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(RO_INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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


void NoRepeatedApplyReadOnlyPolicy::input(SMInput input)
{
	//lyxerr << "NoReapeatedApplyReadOnlyPolicy::input" << endl;

	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_,
			  "NoRepeatedApplyReadOnlyPolicy");
	}
}


/*--------------------------OkApplyCancelReadOnlyPolicy----------------------*/


OkApplyCancelReadOnlyPolicy::OkApplyCancelReadOnlyPolicy()
	: state_(INITIAL),
	  outputs_(RO_APPLIED + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(RO_APPLIED + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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


void OkApplyCancelReadOnlyPolicy::input(SMInput input)
{
	//lyxerr << "OkApplyCancelReadOnlyPolicy::input" << endl;

	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_,
			  "OkApplyCancelReadOnlyPolicy");
	}
}


/*--------------------------OkApplyCancelPolicy----------------------*/


OkApplyCancelPolicy::OkApplyCancelPolicy()
	: state_(INITIAL),
	  outputs_(APPLIED + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(APPLIED + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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


void OkApplyCancelPolicy::input(SMInput input)
{
	//lyxerr << "OkApplyCancelPolicy::input" << endl;

	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_,
			  "OkApplyCancelPolicy");
	}
}


/*--------------------------NoRepeatedApplyPolicy----------------------*/


NoRepeatedApplyPolicy::NoRepeatedApplyPolicy()
	: state_(INITIAL),
	  outputs_(INVALID + 1, ButtonPolicy::ALL_BUTTONS),
	  state_machine_(INVALID + 1,
			 StateArray(int(SMI_TOTAL), ButtonPolicy::BOGUS))
{
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


void NoRepeatedApplyPolicy::input(SMInput input)
{
	//lyxerr << "NoRepeatedApplyPolicy::input" << endl;

	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_,
			  "NoRepeatedApplyPolicy");
	}
}

} // namespace frontend
} // namespace lyx
