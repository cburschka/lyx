// -*- C++ -*-
/**
 * \file FuncStatus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FUNC_STATUS_H
#define FUNC_STATUS_H

#include "support/docstring.h"


namespace lyx {

/// The status of a function.

class FuncStatus
{
private:

	enum StatusCodes {
		/// Command can be executed
		OK = 0,
		/// This command does not exist, possibly because it is not
		/// compiled in (e.g. LFUN_THESAURUS) or the user mistyped
		/// it in the minibuffer. UNKNOWN commands have no menu entry.
		UNKNOWN = 1,
		/// Command cannot be executed
		DISABLED = 2,
		/// Command is on (i. e. the menu item has a checkmark
		/// and the toolbar icon is pushed).
		/// Not all commands use this
		ON = 4,
		/// Command is off (i. e. the menu item has no checkmark
		/// and the toolbar icon is not pushed).
		/// Not all commands use this
		OFF = 8
	};

	unsigned int v_;

	docstring message_;

public:
	///
	FuncStatus();
	///
	void clear();
	///
	void operator|=(FuncStatus const & f);
	///
	void unknown(bool b);
	///
	bool unknown() const;

	///
	void setEnabled(bool b);
	/// tells whether it can be invoked (otherwise it will be grayed-out).
	bool enabled() const;

	///
	void setOnOff(bool b);
	/// tells whether the menu item should have a check mark
	/// (or the toolbar icon should be pushed).
	bool onoff(bool b) const;

	///
	void message(docstring const & m);
	///
	docstring const & message() const;
};


} // namespace lyx

#endif
