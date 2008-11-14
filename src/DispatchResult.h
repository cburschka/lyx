// -*- C++ -*-
/**
 * \file DispatchResult.h
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

#include "update_flags.h"

namespace lyx {

/// Maybe this can go entirely
class DispatchResult {
public:
	///
	DispatchResult() : dispatched_(false), update_(Update::None) {}
	///
	DispatchResult(bool disp, Update::flags f) : dispatched_(disp), update_(f) {}
	//
	bool dispatched() const { return dispatched_; }
	///
	void dispatched(bool disp) { dispatched_ = disp; }
	///
	Update::flags update() const { return update_; }
	///
	void update(Update::flags f) { update_ = f; }
private:
	/// was the event fully dispatched?
	bool dispatched_;
	/// do we need to redraw the screen afterwards?
	Update::flags update_;
};


} // namespace lyx

#endif // DISPATCH_RESULT_H
