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

/// Maybe this can go entirely
class DispatchResult {
public:
	///
	DispatchResult() : dispatched_(false), update_(false) {}
	///
	DispatchResult(bool disp, bool upd) : dispatched_(disp), update_(upd) {}
	//
	bool dispatched() const { return dispatched_; }
	///
	void dispatched(bool disp) { dispatched_ = disp; }
	///
	bool update() const { return update_; }
	///
	void update(bool up) { update_ = up; }
private:
	/// was the event fully dispatched?
	bool dispatched_;
	/// do we need to redraw the screen afterwards?
	bool update_;
};

#endif // DISPATCH_RESULT_H
