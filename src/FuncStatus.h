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

/// The status of a function.

class FuncStatus
{
private:

	enum StatusCodes {
		///
		OK = 0,
		///
		UNKNOWN = 1,
		///
		DISABLED = 2,  // Command cannot be executed
		///
		ON = 4,
		///
		OFF = 8
	};

	unsigned int v_;

public:
	///
	FuncStatus();
	//
	void clear();
	///
	void operator|=(FuncStatus const & f);
	///
	void unknown(bool b);
	///
	bool unknown() const;

	///
	void enabled(bool b);
	///
	bool enabled() const;

	///
	void setOnOff(bool b);
	///
	bool onoff(bool b) const;
};

#endif
