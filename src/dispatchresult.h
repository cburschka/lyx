// -*- C++ -*-
/**
 * \file dispatchresult.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author none
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DISPATCH_RESULT_H
#define DISPATCH_RESULT_H

/** Dispatch result codes
	DISPATCHED          = the inset catched the action
	DISPATCHED_NOUPDATE = the inset catched the action and no update
			is needed here to redraw the inset
	FINISHED            = the inset must be unlocked as a result
			of the action
	FINISHED_RIGHT      = FINISHED, but put the cursor to the RIGHT of
			the inset.
	FINISHED_UP         = FINISHED, but put the cursor UP of
			the inset.
	FINISHED_DOWN       = FINISHED, but put the cursor DOWN of
			the inset.
	UNDISPATCHED        = the action was not catched, it should be
			dispatched by lower level insets
*/
enum dispatch_result_t {
	UNDISPATCHED = 0,
	DISPATCHED,
	DISPATCHED_NOUPDATE,
	FINISHED,
	FINISHED_RIGHT,
	FINISHED_UP,
	FINISHED_DOWN,
	DISPATCHED_POP
};

/** \c DispatchResult is a wrapper for dispatch_result_t.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose insetbase.h.
 */
class DispatchResult {
	dispatch_result_t val_;
public:
	DispatchResult()
		: val_(UNDISPATCHED) {}
	DispatchResult(dispatch_result_t val) : val_(val) {}
	operator dispatch_result_t() const{ return val_; }
};

#endif // DISPATCH_RESULT_H
