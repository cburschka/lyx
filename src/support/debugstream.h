// -*- C++ -*-
/**
 * \file debugstream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DEBUG_STREAM_HPP
#define DEBUG_STREAM_HPP

#include <iostream>

#include <boost/test/detail/nullstream.hpp>

struct debug_trait {
	enum type {
		NONE   = 0,
		EMERG  = 1,
		ALERT  = 2,
		CRIT   = 3,
		ERR    = 4,
		WARN   = 5,
		NOTICE = 6,
		INFO   = 7,
		DEBUG  = 8,
		ANY = 0xffffff
	};

	static bool match(type a, type b) {
		return (b <= a || (b == ANY && a > NONE));
	}
};


template <class dtrait,
	  class charT = char,
	  class traits = std::char_traits<charT> >
class basic_debugstream : public std::basic_ostream<charT, traits> {
public:
	typedef dtrait debug;
	typedef typename debug::type Type;

	basic_debugstream()
		: std::basic_ostream<charT, traits>(0), dt(debug::NONE)
	{}

	/// Constructor, sets the debug level to t.
	explicit basic_debugstream(std::basic_streambuf<charT, traits> * buf)
		: std::basic_ostream<charT, traits>(buf), dt(debug::NONE)
	{}

	/// Sets the debug level to t.
	void level(Type t) {
		dt = t;
	}

	/// Returns the current debug level.
	Type level() const {
		return dt;
	}

	/// Returns true if t is part of the current debug level.
	bool debugging(Type t = debug::ANY) const
	{
		if (debug::match(dt, t)) return true;
		return false;
	}

	/** Returns the no-op stream if t is not part of the
	    current debug level otherwise the real debug stream
	    is used.
	    Use: dbgstream[Debug::INFO] << "Info!\n";
	*/
	std::basic_ostream<charT, traits> & operator[](Type t) {
		if (debug::match(dt, t))
			return *this;
		return nullstream;
	}
private:
	/// The current debug level
	Type dt;
	/// The no-op stream.
	boost::basic_onullstream<charT, traits> nullstream;
};

typedef basic_debugstream<debug_trait> debugstream;

#endif
