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
	DISPATCHED          = the inset caught the action
	DISPATCHED_NOUPDATE = the inset caught the action and no update
			is needed to redraw the inset
	FINISHED            = the inset must be unlocked as a result
			of the action
	FINISHED_RIGHT      = FINISHED, but move the cursor RIGHT from
			the inset.
	FINISHED_UP         = FINISHED, but move the cursor UP from
			the inset.
	FINISHED_DOWN       = FINISHED, but move the cursor DOWN from
			the inset.
	UNDISPATCHED        = the action was not catched, it should be
			dispatched by lower level insets
*/
enum dispatch_result_t {
	NONE = 0,
	NOUPDATE,
	FINISHED,
	FINISHED_RIGHT,
	FINISHED_UP,
	FINISHED_DOWN
};

/** \c DispatchResult is a wrapper for dispatch_result_t.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose insetbase.h.
 */
class DispatchResult {
public:
	DispatchResult()
		: dispatched_(false), val_(NONE) {}
	explicit
	DispatchResult(bool dis)
		: dispatched_(dis), val_(NONE) {}
	DispatchResult(bool dis, dispatch_result_t val)
		: dispatched_(dis), val_(val) {}
	dispatch_result_t val() const { return val_; }
	bool dispatched() const {
		return dispatched_;
	}
private:
	bool dispatched_;
	dispatch_result_t val_;
};


inline
bool operator==(DispatchResult const & lhs, DispatchResult const & rhs)
{
	return lhs.dispatched() == rhs.dispatched() && lhs.val() == rhs.val();
}


inline
bool operator!=(DispatchResult const & lhs, DispatchResult const & rhs)
{
	return !(lhs == rhs);
}

#endif // DISPATCH_RESULT_H
