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
	NONE                = no special action required
	FINISHED_LEFT       = the cursor leaves the inset to the LEFT
	                      as consequence of this action
	FINISHED_RIGHT      = FINISHED, but move the cursor RIGHT from
			the inset.
	FINISHED_UP         = FINISHED, but move the cursor UP from
			the inset.
	FINISHED_DOWN       = FINISHED, but move the cursor DOWN from
			the inset.
*/
enum dispatch_result_t {
	NONE = 0,
	FINISHED_LEFT,
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
		: dispatched_(dis), update_(false), val_(NONE) {}
	DispatchResult(bool dis, bool update)
		: dispatched_(dis), update_(update), val_(NONE) {}
	DispatchResult(bool dis, dispatch_result_t val)
		: dispatched_(dis), update_(true), val_(val) {}
	dispatch_result_t val() const { return val_; }
	void val(dispatch_result_t drt) {
		val_ = drt;
	}
	bool dispatched() const {
		return dispatched_;
	}
	void dispatched(bool dis) {
		dispatched_ = dis;
	}
	bool update() const {
		return update_;
	}
	void update(bool up) {
		update_ = up;
	}
private:
	bool dispatched_;
	bool update_;
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
