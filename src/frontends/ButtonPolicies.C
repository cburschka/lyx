// -*- C++ -*-
/* ButtonPolicies.C
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

#include <config.h>
#include "ButtonPolicies.h"
#include "debug.h"


// Helper function
inline void nextState(ButtonPolicy::State & state,
	       ButtonPolicy::SMInput in,
	       ButtonPolicy::StateMachine s_m,
	       char const * function_name = "nextState")
{
	ButtonPolicy::State tmp = s_m[state][in];
	if (ButtonPolicy::BOGUS != tmp) {
		state = tmp;
	} else {
		lyxerr << function_name
		       << ": No transition for input "
		       << in
		       << " from state "
		       << state
		       << std::endl;
	}
}


/*-----------------------------PreferencesPolicy-----------------------------*/


PreferencesPolicy::PreferencesPolicy()
	: state_(INITIAL),
	  outputs_(APPLIED+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(APPLIED+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = UNDO_ALL | OKAY | APPLY | CANCEL;
	outputs_[INVALID] = UNDO_ALL | CANCEL;
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
	state_machine_[VALID][SMI_UNDO_ALL] = INITIAL;
	// State::INVALID
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_UNDO_ALL] = INITIAL;
	// State::APPLIED
	state_machine_[APPLIED][SMI_VALID] = VALID;
	state_machine_[APPLIED][SMI_INVALID] = INVALID;
	state_machine_[APPLIED][SMI_OKAY] = INITIAL;
	state_machine_[APPLIED][SMI_READ_ONLY] = APPLIED;
	state_machine_[APPLIED][SMI_READ_WRITE] = APPLIED;
}


void PreferencesPolicy::input(SMInput input)
{
	// CANCEL and HIDE always take us to INITIAL for all cases.
	// Note that I didn't put that special case in the helper function
	// because it doesn't belong there.  Some other 
	// This is probably optimising for the wrong case since it occurs as the
	// dialog will be hidden.  It would have saved a little memory in the
	// state machine if I could have gotten map working. ARRae 20000813
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_,
			  input,
			  state_machine_,
			  "PreferencesPolicy");
	}
}


/*-------------------------------OkCancelPolicy------------------------------*/


OkCancelPolicy::OkCancelPolicy()
	: state_(INITIAL),
	  outputs_(INVALID+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(INVALID+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = OKAY | CANCEL;
	outputs_[INVALID] = CANCEL;

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
	// State::INVALID
	state_machine_[INVALID][SMI_VALID] = VALID;
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
}



void OkCancelPolicy::input(SMInput input)
{
	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_, input, state_machine_, "OkCancelPolicy");
	}
}


/*---------------------------OkCancelReadOnlyPolicy-------------------------*/


OkCancelReadOnlyPolicy::OkCancelReadOnlyPolicy()
	: state_(INITIAL),
	  outputs_(RO_INVALID+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(RO_INVALID+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = OKAY | CANCEL;
	outputs_[INVALID] = CANCEL;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = CANCEL;
	outputs_[RO_INVALID] = CANCEL;

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
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
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
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
}


void OkCancelReadOnlyPolicy::input(SMInput input)
{
	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_,
			  input,
			  state_machine_,
			  "OkCancelReadOnlyPolicy");
	}
}


/*--------------------------NoRepeatedApplyReadOnlyPolicy----------------------*/


NoRepeatedApplyReadOnlyPolicy::NoRepeatedApplyReadOnlyPolicy()
	: state_(INITIAL),
	  outputs_(RO_INVALID+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(RO_INVALID+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = OKAY | APPLY | CANCEL;
	outputs_[INVALID] = CANCEL;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = CANCEL;
	outputs_[RO_INVALID] = CANCEL;

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
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
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
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
}


void NoRepeatedApplyReadOnlyPolicy::input(SMInput input)
{
	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_,
			  input,
			  state_machine_,
			  "NoRepeatedApplyReadOnlyPolicy");
	}
}


/*--------------------------OkApplyCancelReadOnlyPolicy----------------------*/


OkApplyCancelReadOnlyPolicy::OkApplyCancelReadOnlyPolicy()
	: state_(INITIAL),
	  outputs_(RO_APPLIED+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(RO_APPLIED+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = OKAY | APPLY | CANCEL;
	outputs_[INVALID] = CANCEL;
	outputs_[APPLIED] = OKAY | APPLY | CLOSE;
	outputs_[RO_INITIAL] = CLOSE;
	outputs_[RO_VALID] = CANCEL;
	outputs_[RO_INVALID] = CANCEL;
	outputs_[RO_APPLIED] = CANCEL;

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
	state_machine_[VALID][SMI_APPLY] = APPLIED;
	state_machine_[VALID][SMI_READ_ONLY] = RO_VALID;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
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
	// State::RO_INVALID
	state_machine_[RO_INVALID][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_READ_ONLY] = RO_INVALID;
	state_machine_[RO_INVALID][SMI_VALID] = RO_VALID;
	state_machine_[RO_INVALID][SMI_READ_WRITE] = INVALID;
	// State::RO_APPLIED
	state_machine_[RO_APPLIED][SMI_READ_ONLY] = RO_APPLIED;
	state_machine_[RO_APPLIED][SMI_INVALID] = RO_INVALID;
	state_machine_[RO_APPLIED][SMI_VALID] = RO_VALID;
	state_machine_[RO_APPLIED][SMI_READ_WRITE] = APPLIED;
}


void OkApplyCancelReadOnlyPolicy::input(SMInput input)
{
	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_,
			  input,
			  state_machine_,
			  "OkApplyCancelReadOnlyPolicy");
	}
}


/*--------------------------OkApplyCancelPolicy----------------------*/


OkApplyCancelPolicy::OkApplyCancelPolicy()
	: state_(INITIAL),
	  outputs_(APPLIED+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(APPLIED+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = OKAY | APPLY | CANCEL;
	outputs_[INVALID] = CANCEL;
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
	state_machine_[VALID][SMI_APPLY] = APPLIED;
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
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
	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_,
			  input,
			  state_machine_,
			  "OkApplyCancelPolicy");
	}
}


/*--------------------------NoRepeatedApplyPolicy----------------------*/


NoRepeatedApplyPolicy::NoRepeatedApplyPolicy()
	: state_(INITIAL),
	  outputs_(INVALID+1, OKAY | APPLY | CANCEL | UNDO_ALL),
	  state_machine_(INVALID+1, StateArray(SMI_TOTAL, BOGUS))
{
	// Build the state output map
	outputs_[INITIAL] = CLOSE;
	outputs_[VALID] = OKAY | APPLY | CANCEL;
	outputs_[INVALID] = CANCEL;

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
	// State::INVALID
	state_machine_[INVALID][SMI_INVALID] = INVALID;
	state_machine_[INVALID][SMI_READ_ONLY] = INVALID;
	state_machine_[INVALID][SMI_READ_WRITE] = INVALID;
	state_machine_[INVALID][SMI_VALID] = VALID;
}


void NoRepeatedApplyPolicy::input(SMInput input)
{
	// CANCEL and HIDE always take us to INITIAL for all cases
	if (SMI_CANCEL == input
	    || SMI_HIDE == input) {
		state_ = INITIAL;
	} else {
		nextState(state_,
			  input,
			  state_machine_,
			  "NoRepeatedApplyPolicy");
	}
}
