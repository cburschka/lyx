// -*- C++ -*-
/**
 * \file xforms/Timeout_pimpl.h
 * Copyright 2001 LyX Team
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */
#ifndef TIMEOUTPIMPL_H
#define TIMEOUTPIMPL_H

#include "frontends/Timeout.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 * This class executes the callback when the timeout expires
 * using XForms mechanisms
 */
struct Timeout::Pimpl {
public:
	///
	Pimpl(Timeout * owner_);
	/// Is the timer running?
	bool running() const;
	/// start the timer
	void start();
	/// stop the timer
	void stop();
	/// reset
	void reset();

private:
	/// the owning timer
	Timeout * owner_;
	/// xforms id
	int timeout_id;
};

#endif
